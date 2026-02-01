// OmniLisp FFI Thread Pool
// Worker threads for async FFI execution
// Based on Purple's threading design

#include <pthread.h>
#include <stdatomic.h>

// =============================================================================
// Configuration
// =============================================================================

#define OMNI_FFI_NUM_WORKERS  4
#define OMNI_FFI_QUEUE_SIZE   256

// =============================================================================
// FFI Call Types
// =============================================================================

typedef enum {
  OMNI_FFI_VOID_VOID = 0,      // void fn(void)
  OMNI_FFI_INT_VOID,           // int fn(void)
  OMNI_FFI_PTR_VOID,           // void* fn(void)
  OMNI_FFI_VOID_INT,           // void fn(int)
  OMNI_FFI_INT_INT,            // int fn(int)
  OMNI_FFI_PTR_INT,            // void* fn(int)
  OMNI_FFI_VOID_PTR,           // void fn(void*)
  OMNI_FFI_INT_PTR,            // int fn(void*)
  OMNI_FFI_PTR_PTR,            // void* fn(void*)
  OMNI_FFI_INT_PTR_INT,        // int fn(void*, int)
  OMNI_FFI_PTR_PTR_INT,        // void* fn(void*, int)
  OMNI_FFI_INT_PTR_PTR,        // int fn(void*, void*)
  OMNI_FFI_PTR_PTR_PTR,        // void* fn(void*, void*)
  OMNI_FFI_VARIADIC,           // General case (slow path)
} OmniFFICallType;

// =============================================================================
// FFI Future (Result Container)
// =============================================================================

typedef struct {
  atomic_int ready;            // 0 = pending, 1 = done
  Term result;                 // Result value
  void *fn_ptr;                // Function pointer
  OmniFFICallType call_type;   // Call signature
  intptr_t args[8];            // Up to 8 arguments
  u32 arg_count;               // Number of arguments
  u32 result_ownership;        // Ownership of result handle
  u32 result_type_id;          // Type ID for result handle
} OmniFFIFuture;

// =============================================================================
// Task Queue
// =============================================================================

typedef struct {
  OmniFFIFuture *tasks[OMNI_FFI_QUEUE_SIZE];
  u32 head;
  u32 tail;
  u32 count;
  pthread_mutex_t mutex;
  pthread_cond_t not_empty;
  pthread_cond_t not_full;
  int shutdown;
} OmniFFIQueue;

// =============================================================================
// Thread Pool
// =============================================================================

typedef struct {
  pthread_t workers[OMNI_FFI_NUM_WORKERS];
  OmniFFIQueue queue;
  int started;
} OmniFFIPool;

// Global pool
static OmniFFIPool OMNI_FFI_POOL = {0};
static int OMNI_FFI_POOL_READY = 0;

// =============================================================================
// Queue Operations
// =============================================================================

fn void omni_ffi_queue_init(OmniFFIQueue *q) {
  q->head = 0;
  q->tail = 0;
  q->count = 0;
  q->shutdown = 0;
  pthread_mutex_init(&q->mutex, NULL);
  pthread_cond_init(&q->not_empty, NULL);
  pthread_cond_init(&q->not_full, NULL);
}

fn void omni_ffi_queue_destroy(OmniFFIQueue *q) {
  pthread_mutex_destroy(&q->mutex);
  pthread_cond_destroy(&q->not_empty);
  pthread_cond_destroy(&q->not_full);
}

fn int omni_ffi_queue_push(OmniFFIQueue *q, OmniFFIFuture *task) {
  pthread_mutex_lock(&q->mutex);

  while (q->count >= OMNI_FFI_QUEUE_SIZE && !q->shutdown) {
    pthread_cond_wait(&q->not_full, &q->mutex);
  }

  if (q->shutdown) {
    pthread_mutex_unlock(&q->mutex);
    return 0;
  }

  q->tasks[q->tail] = task;
  q->tail = (q->tail + 1) % OMNI_FFI_QUEUE_SIZE;
  q->count++;

  pthread_cond_signal(&q->not_empty);
  pthread_mutex_unlock(&q->mutex);
  return 1;
}

fn OmniFFIFuture* omni_ffi_queue_pop(OmniFFIQueue *q) {
  pthread_mutex_lock(&q->mutex);

  while (q->count == 0 && !q->shutdown) {
    pthread_cond_wait(&q->not_empty, &q->mutex);
  }

  if (q->shutdown && q->count == 0) {
    pthread_mutex_unlock(&q->mutex);
    return NULL;
  }

  OmniFFIFuture *task = q->tasks[q->head];
  q->head = (q->head + 1) % OMNI_FFI_QUEUE_SIZE;
  q->count--;

  pthread_cond_signal(&q->not_full);
  pthread_mutex_unlock(&q->mutex);
  return task;
}

// =============================================================================
// FFI Call Execution
// =============================================================================

fn void omni_ffi_execute_call(OmniFFIFuture *f) {
  intptr_t result = 0;

  switch (f->call_type) {
    case OMNI_FFI_VOID_VOID: {
      typedef void (*Fn)(void);
      ((Fn)f->fn_ptr)();
      break;
    }
    case OMNI_FFI_INT_VOID: {
      typedef int (*Fn)(void);
      result = ((Fn)f->fn_ptr)();
      break;
    }
    case OMNI_FFI_PTR_VOID: {
      typedef void* (*Fn)(void);
      result = (intptr_t)((Fn)f->fn_ptr)();
      break;
    }
    case OMNI_FFI_VOID_INT: {
      typedef void (*Fn)(int);
      ((Fn)f->fn_ptr)((int)f->args[0]);
      break;
    }
    case OMNI_FFI_INT_INT: {
      typedef int (*Fn)(int);
      result = ((Fn)f->fn_ptr)((int)f->args[0]);
      break;
    }
    case OMNI_FFI_PTR_INT: {
      typedef void* (*Fn)(int);
      result = (intptr_t)((Fn)f->fn_ptr)((int)f->args[0]);
      break;
    }
    case OMNI_FFI_VOID_PTR: {
      typedef void (*Fn)(void*);
      ((Fn)f->fn_ptr)((void*)f->args[0]);
      break;
    }
    case OMNI_FFI_INT_PTR: {
      typedef int (*Fn)(void*);
      result = ((Fn)f->fn_ptr)((void*)f->args[0]);
      break;
    }
    case OMNI_FFI_PTR_PTR: {
      typedef void* (*Fn)(void*);
      result = (intptr_t)((Fn)f->fn_ptr)((void*)f->args[0]);
      break;
    }
    case OMNI_FFI_INT_PTR_INT: {
      typedef int (*Fn)(void*, int);
      result = ((Fn)f->fn_ptr)((void*)f->args[0], (int)f->args[1]);
      break;
    }
    case OMNI_FFI_PTR_PTR_INT: {
      typedef void* (*Fn)(void*, int);
      result = (intptr_t)((Fn)f->fn_ptr)((void*)f->args[0], (int)f->args[1]);
      break;
    }
    case OMNI_FFI_INT_PTR_PTR: {
      typedef int (*Fn)(void*, void*);
      result = ((Fn)f->fn_ptr)((void*)f->args[0], (void*)f->args[1]);
      break;
    }
    case OMNI_FFI_PTR_PTR_PTR: {
      typedef void* (*Fn)(void*, void*);
      result = (intptr_t)((Fn)f->fn_ptr)((void*)f->args[0], (void*)f->args[1]);
      break;
    }
    case OMNI_FFI_VARIADIC: {
      // General slow path - use libffi or manual dispatch
      // For now, just handle common cases
      break;
    }
  }

  // Store result
  if (f->call_type == OMNI_FFI_VOID_VOID ||
      f->call_type == OMNI_FFI_VOID_INT ||
      f->call_type == OMNI_FFI_VOID_PTR) {
    // Void return - use Nothing
    f->result = term_new_ctr(OMNI_NAM_NOTH, 0, NULL);
  } else if (f->call_type == OMNI_FFI_PTR_VOID ||
             f->call_type == OMNI_FFI_PTR_INT ||
             f->call_type == OMNI_FFI_PTR_PTR ||
             f->call_type == OMNI_FFI_PTR_PTR_INT ||
             f->call_type == OMNI_FFI_PTR_PTR_PTR) {
    // Pointer return - wrap in handle
    if (result == 0) {
      f->result = term_new_ctr(OMNI_NAM_NOTH, 0, NULL);
    } else {
      f->result = omni_ffi_handle_alloc(
        (void*)result,
        (OmniOwnership)f->result_ownership,
        f->result_type_id
      );
    }
  } else {
    // Integer return - wrap as Cst (NUM)
    f->result = term_new_num((u32)result);
  }

  // Mark as ready (release fence)
  atomic_store_explicit(&f->ready, 1, memory_order_release);
}

// =============================================================================
// Worker Thread
// =============================================================================

fn void* omni_ffi_worker(void *arg) {
  (void)arg;

  while (1) {
    OmniFFIFuture *task = omni_ffi_queue_pop(&OMNI_FFI_POOL.queue);
    if (!task) break;  // Shutdown

    omni_ffi_execute_call(task);
  }

  return NULL;
}

// =============================================================================
// Pool Initialization
// =============================================================================

fn void omni_ffi_pool_init(void) {
  if (OMNI_FFI_POOL_READY) return;

  omni_ffi_queue_init(&OMNI_FFI_POOL.queue);

  for (int i = 0; i < OMNI_FFI_NUM_WORKERS; i++) {
    pthread_create(&OMNI_FFI_POOL.workers[i], NULL, omni_ffi_worker, NULL);
  }

  OMNI_FFI_POOL.started = 1;
  OMNI_FFI_POOL_READY = 1;
}

// =============================================================================
// Pool Shutdown
// =============================================================================

fn void omni_ffi_pool_shutdown(void) {
  if (!OMNI_FFI_POOL_READY) return;

  // Signal shutdown
  pthread_mutex_lock(&OMNI_FFI_POOL.queue.mutex);
  OMNI_FFI_POOL.queue.shutdown = 1;
  pthread_cond_broadcast(&OMNI_FFI_POOL.queue.not_empty);
  pthread_mutex_unlock(&OMNI_FFI_POOL.queue.mutex);

  // Wait for workers
  for (int i = 0; i < OMNI_FFI_NUM_WORKERS; i++) {
    pthread_join(OMNI_FFI_POOL.workers[i], NULL);
  }

  omni_ffi_queue_destroy(&OMNI_FFI_POOL.queue);
  OMNI_FFI_POOL_READY = 0;
}

// =============================================================================
// Async FFI Dispatch
// =============================================================================

// Submit an FFI call and return a pending future
fn Term omni_ffi_call_async(
  void *fn_ptr,
  OmniFFICallType call_type,
  intptr_t *args,
  u32 arg_count,
  OmniOwnership result_ownership,
  u32 result_type_id
) {
  if (!OMNI_FFI_POOL_READY) omni_ffi_pool_init();

  OmniFFIFuture *f = (OmniFFIFuture*)calloc(1, sizeof(OmniFFIFuture));
  f->fn_ptr = fn_ptr;
  f->call_type = call_type;
  f->arg_count = arg_count;
  for (u32 i = 0; i < arg_count && i < 8; i++) {
    f->args[i] = args[i];
  }
  f->result_ownership = result_ownership;
  f->result_type_id = result_type_id;
  atomic_store(&f->ready, 0);

  // Submit to queue
  omni_ffi_queue_push(&OMNI_FFI_POOL.queue, f);

  // Return #Pend{future_ptr}
  return omni_ffi_ptr_wrap((void*)f);
}

// =============================================================================
// Sync FFI Call (blocking)
// =============================================================================

fn Term omni_ffi_call_sync(
  void *fn_ptr,
  OmniFFICallType call_type,
  intptr_t *args,
  u32 arg_count,
  OmniOwnership result_ownership,
  u32 result_type_id
) {
  OmniFFIFuture f = {0};
  f.fn_ptr = fn_ptr;
  f.call_type = call_type;
  f.arg_count = arg_count;
  for (u32 i = 0; i < arg_count && i < 8; i++) {
    f.args[i] = args[i];
  }
  f.result_ownership = result_ownership;
  f.result_type_id = result_type_id;

  omni_ffi_execute_call(&f);
  return f.result;
}

// =============================================================================
// Await Future
// =============================================================================

fn Term omni_ffi_await(Term pending) {
  // #Ptr{hi, lo} is C02 (2 args)
  if (term_tag(pending) != C02) return pending;
  if (term_ext(pending) != OMNI_NAM_PEND &&
      term_ext(pending) != OMNI_NAM_PTR) return pending;

  OmniFFIFuture *f = (OmniFFIFuture*)omni_ffi_ptr_unwrap(pending);
  if (!f) return term_new_ctr(OMNI_NAM_ERR, 0, NULL);

  // Spin wait with backoff
  u32 spins = 0;
  while (!atomic_load_explicit(&f->ready, memory_order_acquire)) {
    if (spins < 1000) {
      spins++;
      // Busy spin
    } else if (spins < 10000) {
      spins++;
      sched_yield();
    } else {
      // Sleep a bit
      struct timespec ts = {0, 100000};  // 100us
      nanosleep(&ts, NULL);
    }
  }

  Term result = f->result;
  free(f);
  return result;
}

// =============================================================================
// FFI Function Table
// =============================================================================

#define OMNI_FFI_TABLE_SIZE 256

typedef struct {
  u32 name_nick;               // Nick-encoded function name
  void *fn_ptr;                // Function pointer
  OmniFFICallType call_type;   // Signature type
  OmniOwnership result_ownership;
  u32 result_type_id;
} OmniFFIEntry;

static OmniFFIEntry OMNI_FFI_TABLE[OMNI_FFI_TABLE_SIZE];
static u32 OMNI_FFI_TABLE_COUNT = 0;

// Register an FFI function
fn void omni_ffi_register(
  const char *name,
  void *fn_ptr,
  OmniFFICallType call_type,
  OmniOwnership result_ownership,
  u32 result_type_id
) {
  if (OMNI_FFI_TABLE_COUNT >= OMNI_FFI_TABLE_SIZE) return;

  OmniFFIEntry *e = &OMNI_FFI_TABLE[OMNI_FFI_TABLE_COUNT++];
  e->name_nick = omni_nick(name);
  e->fn_ptr = fn_ptr;
  e->call_type = call_type;
  e->result_ownership = result_ownership;
  e->result_type_id = result_type_id;
}

// Lookup FFI function by nick
fn OmniFFIEntry* omni_ffi_lookup(u32 name_nick) {
  for (u32 i = 0; i < OMNI_FFI_TABLE_COUNT; i++) {
    if (OMNI_FFI_TABLE[i].name_nick == name_nick) {
      return &OMNI_FFI_TABLE[i];
    }
  }
  return NULL;
}

// =============================================================================
// FFI Dispatch (called during reduction)
// =============================================================================

// Dispatch #FFI{name, args} node
fn Term omni_ffi_dispatch(Term ffi_node) {
  // #FFI{name, args} is C02 (2 args)
  if (term_tag(ffi_node) != C02) return ffi_node;
  if (term_ext(ffi_node) != OMNI_NAM_FFI) return ffi_node;

  u32 loc = term_val(ffi_node);

  // CRITICAL: Reduce the name term to resolve ALO markers to actual values
  Term name_term = wnf(HEAP[loc]);

  // The name should be a NUM with the nick
  u32 name_nick = term_val(name_term);

  // Reduce args list too
  Term args_list = wnf(HEAP[loc + 1]);

  // Try IO dispatch first (omni_ffi_io_dispatch is defined in io.c)
  Term io_result = omni_ffi_io_dispatch(name_nick, args_list);
  if (io_result != 0) {
    return io_result;
  }

  // Try DateTime dispatch (omni_ffi_dt_dispatch is defined in datetime.c)
  Term dt_result = omni_ffi_dt_dispatch(name_nick, args_list);
  if (dt_result != 0) {
    return dt_result;
  }

  // Try JSON dispatch (omni_ffi_json_dispatch is defined in json.c)
  Term json_result = omni_ffi_json_dispatch(name_nick, args_list);
  if (json_result != 0) {
    return json_result;
  }

  OmniFFIEntry *entry = omni_ffi_lookup(name_nick);
  if (!entry) {
    return term_new_ctr(OMNI_NAM_ERR, 0, NULL);
  }

  // Extract arguments from cons list
  intptr_t args[8] = {0};
  u32 arg_count = 0;
  Term cur = args_list;

  // #CON{h, t} is C02 (2 args)
  while (term_tag(cur) == C02 && term_ext(cur) == NAM_CON && arg_count < 8) {
    u32 aloc = term_val(cur);
    // Reduce each element before reading
    Term head = wnf(HEAP[aloc]);
    cur = wnf(HEAP[aloc + 1]);

    // Convert Term to intptr_t
    if (term_tag(head) == C01) {
      // 1-arg constructors: #Cst{n}, #Hndl{packed}
      if (term_ext(head) == OMNI_NAM_CST) {
        // Reduce the inner value too
        Term inner = wnf(HEAP[term_val(head)]);
        args[arg_count++] = (intptr_t)term_val(inner);
      } else if (term_ext(head) == OMNI_NAM_HNDL) {
        void *ptr = omni_ffi_handle_borrow(head);
        args[arg_count++] = (intptr_t)ptr;
      }
    } else if (term_tag(head) == C02) {
      // 2-arg constructors: #Ptr{hi, lo}
      if (term_ext(head) == OMNI_NAM_PTR) {
        void *ptr = omni_ffi_ptr_unwrap(head);
        args[arg_count++] = (intptr_t)ptr;
      }
    } else if (term_tag(head) == NUM) {
      args[arg_count++] = (intptr_t)term_val(head);
    }
  }

  // Execute synchronously (async could be selected based on metadata)
  return omni_ffi_call_sync(
    entry->fn_ptr,
    entry->call_type,
    args,
    arg_count,
    entry->result_ownership,
    entry->result_type_id
  );
}

// =============================================================================
// Standard Library FFI Registration
// =============================================================================

fn void omni_ffi_register_stdlib(void) {
  // Memory
  omni_ffi_register("mloc", (void*)malloc, OMNI_FFI_PTR_INT, OMNI_OWNED, 0);
  omni_ffi_register("free", (void*)free, OMNI_FFI_VOID_PTR, OMNI_BORROWED, 0);
  omni_ffi_register("rloc", (void*)realloc, OMNI_FFI_PTR_PTR_INT, OMNI_OWNED, 0);
  omni_ffi_register("cloc", (void*)calloc, OMNI_FFI_VARIADIC, OMNI_OWNED, 0);

  // I/O
  omni_ffi_register("puts", (void*)puts, OMNI_FFI_INT_PTR, OMNI_BORROWED, 0);
  omni_ffi_register("putc", (void*)putchar, OMNI_FFI_INT_INT, OMNI_BORROWED, 0);
  omni_ffi_register("getc", (void*)getchar, OMNI_FFI_INT_VOID, OMNI_BORROWED, 0);

  // File I/O
  omni_ffi_register("fopn", (void*)fopen, OMNI_FFI_PTR_PTR_PTR, OMNI_OWNED, 0);
  omni_ffi_register("fcls", (void*)fclose, OMNI_FFI_INT_PTR, OMNI_BORROWED, 0);
  omni_ffi_register("frd", (void*)fread, OMNI_FFI_VARIADIC, OMNI_BORROWED, 0);
  omni_ffi_register("fwrt", (void*)fwrite, OMNI_FFI_VARIADIC, OMNI_BORROWED, 0);
}

