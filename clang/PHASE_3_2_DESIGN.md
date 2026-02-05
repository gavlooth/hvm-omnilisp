# Phase 3.2: Effect Handlers

**Status:** Design Phase  
**Date:** 2026-02-05  
**Depends on:** Phase 3.1 (Effect Row Types)

## Overview

Effect handlers implement algebraic effects with first-class continuations. They provide a structured way to handle computational effects like state, exceptions, non-determinism, and custom control flow.

## Background

**Algebraic Effects** are a programming language feature that separates:
- **Effect operations** (what to do) - via `perform`
- **Effect handlers** (how to do it) - via `handle`

This separation enables:
- Modular effect composition
- Multi-shot continuations (resuming multiple times)
- Effect-specific optimizations
- Clear separation of concerns

## Current Implementation

Phase 2.4 includes runtime effect handling:

```scheme
(handle
  (perform get-value)
  (get-value [resume] (resume 42)))
```

Phase 3.2 extends this with:
- Static verification (using Phase 3.1 effect rows)
- Multi-shot continuations
- Handler composition
- Effect transformation

## Core Concepts

### 1. Effect Operations

```scheme
;; Define an effect operation
(define-effect State
  (get [] Int)
  (put [Int] Unit))

;; Perform an effect
(perform (State/get))      ;; Returns current state
(perform (State/put 42))   ;; Sets state to 42
```

### 2. Effect Handlers

```scheme
;; Handler for State effect
(handle-state [initial-state] [body]
  (handle body
    (State/get [resume]
      ;; resume with current state
      (resume state))
    (State/put [new-state resume]
      ;; update state and continue
      (resume new-state unit))))
```

### 3. Continuation Operations

**One-shot (linear):**
```scheme
(handle
  (+ 1 (perform get-value))
  (get-value [k] 
    (k 41)))  ;; Resume once: 42
```

**Multi-shot (non-linear):**
```scheme
(handle
  (perform choose)
  (choose [k]
    (cons (k true) (cons (k false) ()))))
;; Resume twice, collecting both results
```

**Zero-shot (abort):**
```scheme
(handle
  (perform fail "error")
  (fail [msg k]
    none))  ;; Never resume, return none
```

## Handler Patterns

### Pattern 1: State Handler

```scheme
(define handle-state [init] [body]
  (letrec 
    ([loop (lambda [state]
      (handle body
        (get [k] (k state state))
        (put [s k] (k s unit))))])
    (loop init)))

;; Usage
(handle-state 0
  (do
    (perform put 10)
    (let [x (perform get)]
      (+ x 5))))
;; Returns: 15
```

### Pattern 2: Exception Handler

```scheme
(define handle-exception [body]
  (handle body
    (raise [msg k] 
      {:error true :message msg})))

;; Usage
(handle-exception
  (if (< x 0)
    (perform raise "negative value")
    (sqrt x)))
```

### Pattern 3: Non-determinism Handler

```scheme
(define handle-all [body]
  (handle body
    (choose [k]
      (append (k true) (k false)))))

;; Usage  
(handle-all
  (let [x (perform choose)]
    (let [y (perform choose)]
      (list x y))))
;; Returns: [(true true) (true false) (false true) (false false)]
```

### Pattern 4: Reader (Environment) Handler

```scheme
(define handle-reader [env] [body]
  (handle body
    (ask [k] (k env env))))

;; Usage
(handle-reader {:debug true :timeout 30}
  (let [cfg (perform ask)]
    (if (:debug cfg)
      (perform log "debug mode")
      unit)))
```

### Pattern 5: Writer (Logging) Handler

```scheme
(define handle-writer [body]
  (letrec
    ([loop (lambda [log]
      (handle body
        (tell [msg k] 
          (k (cons msg log) unit))))])
    (reverse (loop ()))))

;; Usage
(handle-writer
  (do
    (perform tell "start")
    (let [x (+ 1 2)]
      (perform tell (format "result: ~a" x))
      x)))
;; Returns: (3, ["start" "result: 3"])
```

## Handler Composition

### Sequential Composition

```scheme
(handle-exception
  (handle-state 0
    (do
      (if (< (perform get) 0)
        (perform raise "negative state")
        (perform put 42)))))
```

### Monad Transformer Pattern

```scheme
;; StateT Exception
(define handle-state-exception [init] [body]
  (handle-exception
    (handle-state init
      body)))
```

## Advanced Features

### 1. Parameterized Handlers

```scheme
(define handle-state-with-log [init] [body]
  (handle-writer
    (handle-state init
      (do
        (perform tell "state initialized")
        body))))
```

### 2. Dynamic Effect Binding

```scheme
(define with-timeout [seconds] [body]
  (handle body
    (async [k]
      (schedule-timeout seconds (lambda [] (k ())))
      (k ()))))
```

### 3. Effect Forwarding

```scheme
(define handle-logged-state [init] [body]
  (handle-state init
    (handle body
      (get [k] 
        (do
          (perform tell "reading state")
          (k (perform get))))
      (put [s k]
        (do
          (perform tell (format "writing state: ~a" s))
          (k (perform put s)))))))
```

## Implementation Strategy

### Phase 3.2.1: Continuation Representation

```c
typedef struct Continuation {
    Term *hole;          // Term with a hole to fill
    Environment *env;    // Captured environment
    EffectRow *effects;  // Remaining effects
} Continuation;

// Create continuation from evaluation context
Continuation *capture_continuation(EvalContext *ctx);

// Resume continuation with value
Term *resume_continuation(Continuation *k, Term *value);
```

### Phase 3.2.2: Handler Runtime

```c
typedef struct Handler {
    char *effect_name;
    Term *handler_fn;    // (lambda [payload resume] body)
    Handler *next;       // Handler chain
} Handler;

// Install handler
void push_handler(EvalContext *ctx, Handler *h);

// Perform effect operation
Term *perform_effect(EvalContext *ctx, 
                     const char *effect,
                     Term *payload);
```

### Phase 3.2.3: Multi-shot Support

```c
typedef struct MultiShot {
    Continuation *k;
    int resume_count;
    Term **results;      // Collect results from resumes
    size_t result_cap;
} MultiShot;

// Enable multi-shot for handler
MultiShot *make_multishot(Continuation *k);

// Resume and collect result
void multishot_resume(MultiShot *ms, Term *value);
```

### Phase 3.2.4: Handler Verification

Using Phase 3.1 effect rows:

```c
// Check handler covers all performed effects
bool verify_handler_coverage(Term *body_effects,
                             Handler *handlers);

// Infer remaining effects after handling
EffectRow *compute_residual_effects(EffectRow *body_effects,
                                    Handler *handlers);
```

## Test Coverage

Test files validating effect handlers:

- `test_effects.omni` - Basic handler patterns
- `test_effects_state.omni` - State effect handler
- `test_effects_exception.omni` - Exception handling
- `test_effects_choice.omni` - Non-determinism
- `test_effects_reader.omni` - Reader/environment
- `test_effects_writer.omni` - Writer/logging
- `test_effects_nested.omni` - Handler composition
- `test_effects_multi_resume.omni` - Multi-shot continuations
- `test_effects_compose.omni` - Effect composition
- `test_effects_finally.omni` - Cleanup/finalization

## Examples

### Example 1: Counting Effect Uses

```scheme
(define count-reads [body]
  (letrec
    ([loop (lambda [count]
      (handle body
        (read [k]
          (let [result (k some-value)]
            (loop (+ count 1))))))])
    (loop 0)))

(count-reads
  (do
    (perform read)
    (perform read)
    (perform read)))
;; Returns: 3
```

### Example 2: Transaction Handler

```scheme
(define handle-transaction [body]
  (handle body
    (abort [k] (rollback) none)
    (commit [k] (finalize) (k () unit))))

(handle-transaction
  (do
    (perform write key1 val1)
    (if (valid? val1)
      (perform commit)
      (perform abort))))
```

### Example 3: Async/Await Pattern

```scheme
(define handle-async [body]
  (handle body
    (await [promise k]
      (schedule-callback promise
        (lambda [result] (k result))))))

(handle-async
  (let [data (perform await (fetch-url "http://..."))]
    (process data)))
```

## Integration with Phase 3.1

Effect handlers use effect row types for verification:

```scheme
;; Function signature with effects
get-config : Unit -> Config / {ReadConfig}

;; Handler removes effect from type
handle config-handler body : Config / {}
  where body : Config / {ReadConfig}
```

## Success Criteria

- [ ] Continuation capture and resume
- [ ] Effect handler installation and lookup
- [ ] Multi-shot continuation support
- [ ] Handler composition works correctly
- [ ] All test files pass
- [ ] Zero-shot (abort) supported
- [ ] Handler verification using effect rows

## Next Phase

**Phase 3.3: Effect Inference** will implement:
- Automatic effect inference from function bodies
- Effect row minimization
- Effect polymorphism constraints

---

**Implementation Approach:** Design specification for HVM backend. C interpreter maintains runtime-only handler dispatch.

---

**⚠️ HVM4 IMPLEMENTATION ONLY - DO NOT IMPLEMENT IN C INTERPRETER ⚠️**
