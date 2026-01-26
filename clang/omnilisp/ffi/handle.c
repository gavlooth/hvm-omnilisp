// OmniLisp Handle Table
// Safe handle-based memory management for FFI pointers
// Uses generation counters for ABA protection

#include "../../../hvm4/clang/hvm4.c"

// =============================================================================
// Ownership Kinds
// =============================================================================

typedef enum {
  OMNI_OWNED     = 0,  // Runtime owns, will free on drop
  OMNI_BORROWED  = 1,  // Borrowed reference, must not free
  OMNI_CONSUMED  = 2,  // Ownership transferred to C, handle invalidated
  OMNI_ESCAPES   = 3,  // Pointer escapes to unknown scope
} OmniOwnership;

// =============================================================================
// Handle Slot
// =============================================================================

typedef struct {
  u32  generation;     // Generation counter for ABA protection
  void *pointer;       // The actual C pointer
  OmniOwnership ownership;
  u32  type_id;        // Type identifier for runtime checks
  u32  next_free;      // Index of next free slot (when in free list)
} OmniHandleSlot;

// =============================================================================
// Handle Table
// =============================================================================

#define OMNI_HANDLE_INITIAL_CAP 1024
#define OMNI_HANDLE_MAX_CAP     (1 << 20)  // 1M handles max

typedef struct {
  OmniHandleSlot *slots;
  u32  capacity;
  u32  free_head;      // Head of free list (UINT32_MAX = none free)
  u32  count;          // Number of allocated handles
} OmniHandleTable;

// Global handle table
static OmniHandleTable OMNI_HANDLES = {0};
static int OMNI_HANDLES_READY = 0;

// =============================================================================
// Handle Table Initialization
// =============================================================================

fn void omni_ffi_handle_init(void) {
  if (OMNI_HANDLES_READY) return;

  OMNI_HANDLES.capacity = OMNI_HANDLE_INITIAL_CAP;
  OMNI_HANDLES.slots = (OmniHandleSlot*)calloc(
    OMNI_HANDLE_INITIAL_CAP, sizeof(OmniHandleSlot)
  );

  // Build initial free list
  for (u32 i = 0; i < OMNI_HANDLE_INITIAL_CAP - 1; i++) {
    OMNI_HANDLES.slots[i].next_free = i + 1;
    OMNI_HANDLES.slots[i].generation = 0;
    OMNI_HANDLES.slots[i].pointer = NULL;
  }
  OMNI_HANDLES.slots[OMNI_HANDLE_INITIAL_CAP - 1].next_free = UINT32_MAX;
  OMNI_HANDLES.free_head = 0;
  OMNI_HANDLES.count = 0;

  OMNI_HANDLES_READY = 1;
}

// =============================================================================
// Handle Table Growth
// =============================================================================

fn int omni_ffi_handle_grow(void) {
  u32 new_cap = OMNI_HANDLES.capacity * 2;
  if (new_cap > OMNI_HANDLE_MAX_CAP) {
    return 0;  // Cannot grow further
  }

  OmniHandleSlot *new_slots = (OmniHandleSlot*)realloc(
    OMNI_HANDLES.slots,
    new_cap * sizeof(OmniHandleSlot)
  );
  if (!new_slots) return 0;

  // Initialize new slots and add to free list
  for (u32 i = OMNI_HANDLES.capacity; i < new_cap - 1; i++) {
    new_slots[i].next_free = i + 1;
    new_slots[i].generation = 0;
    new_slots[i].pointer = NULL;
  }
  new_slots[new_cap - 1].next_free = OMNI_HANDLES.free_head;
  OMNI_HANDLES.free_head = OMNI_HANDLES.capacity;

  OMNI_HANDLES.slots = new_slots;
  OMNI_HANDLES.capacity = new_cap;

  return 1;
}

// =============================================================================
// Handle Allocation
// =============================================================================

// Allocate a handle for a pointer
// Returns handle as Term: #Hndl{idx, gen}
fn Term omni_ffi_handle_alloc(void *ptr, OmniOwnership ownership, u32 type_id) {
  if (!OMNI_HANDLES_READY) omni_ffi_handle_init();

  // Grow table if needed
  if (OMNI_HANDLES.free_head == UINT32_MAX) {
    if (!omni_ffi_handle_grow()) {
      // Out of handles - return error
      return term_new(CTR, OMNI_NAM_ERR, 0);
    }
  }

  // Pop from free list
  u32 idx = OMNI_HANDLES.free_head;
  OmniHandleSlot *slot = &OMNI_HANDLES.slots[idx];
  OMNI_HANDLES.free_head = slot->next_free;
  OMNI_HANDLES.count++;

  // Initialize slot
  slot->pointer = ptr;
  slot->ownership = ownership;
  slot->type_id = type_id;
  // generation already set (preserved across reuse)

  // Return #Hndl{idx, gen} as a CTR node
  // Pack idx (20 bits) and gen (12 bits) into val
  u32 packed = (idx & 0xFFFFF) | ((slot->generation & 0xFFF) << 20);
  return term_new(CTR, OMNI_NAM_HNDL, heap_alloc(1, &packed));
}

// =============================================================================
// Handle Deallocation
// =============================================================================

// Free a handle and optionally the underlying pointer
fn int omni_ffi_handle_free(Term handle) {
  if (term_tag(handle) != CTR) return 0;
  if (term_ext(handle) != OMNI_NAM_HNDL) return 0;

  u32 packed = heap[term_val(handle)];
  u32 idx = packed & 0xFFFFF;
  u32 gen = (packed >> 20) & 0xFFF;

  if (idx >= OMNI_HANDLES.capacity) return 0;

  OmniHandleSlot *slot = &OMNI_HANDLES.slots[idx];

  // Validate generation
  if (slot->generation != gen) {
    return 0;  // Stale handle
  }

  // Free the underlying pointer if owned
  if (slot->ownership == OMNI_OWNED && slot->pointer) {
    free(slot->pointer);
  }

  // Increment generation to invalidate any remaining references
  slot->generation = (slot->generation + 1) & 0xFFF;
  slot->pointer = NULL;
  slot->ownership = OMNI_BORROWED;
  slot->type_id = 0;

  // Push to free list
  slot->next_free = OMNI_HANDLES.free_head;
  OMNI_HANDLES.free_head = idx;
  OMNI_HANDLES.count--;

  return 1;
}

// =============================================================================
// Handle Dereferencing
// =============================================================================

// Get the pointer from a handle (with validation)
fn void* omni_ffi_handle_deref(Term handle) {
  if (term_tag(handle) != CTR) return NULL;
  if (term_ext(handle) != OMNI_NAM_HNDL) return NULL;

  u32 packed = heap[term_val(handle)];
  u32 idx = packed & 0xFFFFF;
  u32 gen = (packed >> 20) & 0xFFF;

  if (idx >= OMNI_HANDLES.capacity) return NULL;

  OmniHandleSlot *slot = &OMNI_HANDLES.slots[idx];

  // Validate generation
  if (slot->generation != gen) {
    return NULL;  // Stale handle
  }

  return slot->pointer;
}

// Get handle slot info (for ownership checking)
fn OmniHandleSlot* omni_ffi_handle_slot(Term handle) {
  if (term_tag(handle) != CTR) return NULL;
  if (term_ext(handle) != OMNI_NAM_HNDL) return NULL;

  u32 packed = heap[term_val(handle)];
  u32 idx = packed & 0xFFFFF;
  u32 gen = (packed >> 20) & 0xFFF;

  if (idx >= OMNI_HANDLES.capacity) return NULL;

  OmniHandleSlot *slot = &OMNI_HANDLES.slots[idx];

  if (slot->generation != gen) {
    return NULL;
  }

  return slot;
}

// =============================================================================
// Handle Validation
// =============================================================================

fn int omni_ffi_handle_valid(Term handle) {
  return omni_ffi_handle_deref(handle) != NULL;
}

// Check if handle has expected type
fn int omni_ffi_handle_type_check(Term handle, u32 expected_type) {
  OmniHandleSlot *slot = omni_ffi_handle_slot(handle);
  if (!slot) return 0;
  return slot->type_id == expected_type;
}

// =============================================================================
// Ownership Transfer
// =============================================================================

// Mark handle as consumed (ownership transferred to C)
fn int omni_ffi_handle_consume(Term handle) {
  OmniHandleSlot *slot = omni_ffi_handle_slot(handle);
  if (!slot) return 0;

  if (slot->ownership != OMNI_OWNED) {
    return 0;  // Can only consume owned handles
  }

  slot->ownership = OMNI_CONSUMED;

  // Invalidate handle
  slot->generation = (slot->generation + 1) & 0xFFF;
  slot->pointer = NULL;  // C now owns this

  // Return to free list
  slot->next_free = OMNI_HANDLES.free_head;
  OMNI_HANDLES.free_head = (u32)(slot - OMNI_HANDLES.slots);
  OMNI_HANDLES.count--;

  return 1;
}

// Borrow a handle (for FFI call that doesn't take ownership)
fn void* omni_ffi_handle_borrow(Term handle) {
  OmniHandleSlot *slot = omni_ffi_handle_slot(handle);
  if (!slot) return NULL;

  // Borrowed access is always allowed
  return slot->pointer;
}

// =============================================================================
// Handle Table Cleanup
// =============================================================================

fn void omni_ffi_handle_cleanup(void) {
  if (!OMNI_HANDLES_READY) return;

  // Free all owned pointers
  for (u32 i = 0; i < OMNI_HANDLES.capacity; i++) {
    OmniHandleSlot *slot = &OMNI_HANDLES.slots[i];
    if (slot->pointer && slot->ownership == OMNI_OWNED) {
      free(slot->pointer);
      slot->pointer = NULL;
    }
  }

  free(OMNI_HANDLES.slots);
  OMNI_HANDLES.slots = NULL;
  OMNI_HANDLES.capacity = 0;
  OMNI_HANDLES.free_head = UINT32_MAX;
  OMNI_HANDLES.count = 0;
  OMNI_HANDLES_READY = 0;
}

// =============================================================================
// Pointer Wrapping Utilities
// =============================================================================

// Wrap a raw pointer as a Term (for 64-bit pointers)
// Returns #Ptr{hi, lo} where hi and lo are 32-bit halves
fn Term omni_ffi_ptr_wrap(void *ptr) {
  uintptr_t p = (uintptr_t)ptr;
  u32 lo = (u32)(p & 0xFFFFFFFF);
  u32 hi = (u32)(p >> 32);

  u32 loc = heap_alloc(2, NULL);
  heap[loc] = hi;
  heap[loc + 1] = lo;

  return term_new(CTR, OMNI_NAM_PTR, loc);
}

// Unwrap a #Ptr{hi, lo} to a raw pointer
fn void* omni_ffi_ptr_unwrap(Term ptr_term) {
  if (term_tag(ptr_term) != CTR) return NULL;
  if (term_ext(ptr_term) != OMNI_NAM_PTR) return NULL;

  u32 loc = term_val(ptr_term);
  u32 hi = heap[loc];
  u32 lo = heap[loc + 1];

  uintptr_t p = ((uintptr_t)hi << 32) | (uintptr_t)lo;
  return (void*)p;
}

// =============================================================================
// Handle Statistics
// =============================================================================

fn u32 omni_ffi_handle_count(void) {
  return OMNI_HANDLES.count;
}

fn u32 omni_ffi_handle_capacity(void) {
  return OMNI_HANDLES.capacity;
}

