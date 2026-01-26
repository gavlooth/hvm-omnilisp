# OmniLisp Implementation Comparison

## Code Size

| Component | Original OmniLisp | HVM4 OmniLisp |
|-----------|-------------------|---------------|
| Compiler (C) | ~56,000 lines | ~1,400 lines |
| Runtime (C) | ~150,000 lines | N/A (uses HVM4) |
| Runtime (HVM4) | N/A | ~920 lines |
| **Total** | **~206,000 lines** | **~2,300 lines** |

## Feature Comparison

### Implemented in HVM4 Version

| Feature | Status | Notes |
|---------|--------|-------|
| Core evaluator | ✅ | Direct + CPS modes |
| Lambda/closures | ✅ | Including recursive |
| Let bindings | ✅ | Basic |
| Conditionals (if) | ✅ | |
| Arithmetic (+,-,*,/,%) | ✅ | |
| Comparisons (=,<,>) | ✅ | |
| Pattern matching | ✅ | Basic patterns |
| Delimited continuations | ✅ | reset/control/shift |
| Algebraic effects | ✅ | handle/perform with CPS |
| Fibers | ✅ | CPS-based continuations |
| Fork/choice | ✅ | HVM4 SUP-based |
| Basic FFI | ✅ | Handle table, thread pool |

### Missing from HVM4 Version

| Feature | Original | Priority |
|---------|----------|----------|
| **Type System** | Full inference, annotations, checking | High |
| **Gradual Multiple Dispatch** | Arity + type-based, compile-time when typed | High |
| **Macro System** | Hygienic macros, syntax-rules | Medium |
| **Channels (CSP)** | Buffered/unbuffered, send/recv | Optional (defer) |
| **Destructuring** | In let, match, function params | Medium |
| **Collections** | Arrays, dicts, sets | High |
| **Iterators** | Lazy map/filter/range | Medium |
| **I/O System** | File, network, console | High |
| **Regex** | PEG-based pattern matching | Low |
| **JSON** | Parse/stringify | Medium |
| **Modules** | Import/export, namespaces | Medium |
| **Conditions/Restarts** | Common Lisp style | Low |
| **String Operations** | concat, split, format | High |
| **Math Library** | sin, cos, sqrt, etc. | Medium |
| **Quasiquote** | `, ,@ syntax | Medium |
| **Default Parameters** | `[x {Int} 10]` | Low |
| **Variadic Functions** | `.. rest` spread pattern | Medium |
| **Region Memory** | ASAP, RC-G | N/A (HVM4 handles) |
| **Profiling** | Timing, allocation stats | Low |
| **Generic Functions** | CLOS-style dispatch | Medium |

### Architectural Differences

| Aspect | Original | HVM4 |
|--------|----------|------|
| Memory | Region-based RC, ASAP | HVM4 interaction nets |
| Parallelism | pthreads + fibers | HVM4 native parallelism (SUP) |
| Compilation | C codegen | HVM4 term generation |
| Continuations | CPS + trampoline | Native HVM4 lambdas |
| Effects | Stack-based handlers | CPS capture |

## Core Primitive Discrepancies

### 1. Continuation Representation

| Aspect | Original OmniLisp | HVM4 OmniLisp |
|--------|-------------------|---------------|
| **Structure** | Frame chain with 11 frame types (APP_FN, APP_ARGS, APP_DONE, IF_TEST, LET_BIND, LET_BODY, SEQ, PROMPT, HANDLER, YIELD, AWAIT) | Native HVM4 lambda (`#Kont{k}`) |
| **Reference Counting** | Explicit `refcount` field on frames and continuations | HVM4 automatic (interaction nets) |
| **Frame Pooling** | Pre-allocated frame pool for hot path allocation | N/A (HVM4 handles allocation) |
| **Memory Location** | Heap-allocated frame chain | Closure environment capture |

**Impact**: The HVM4 version is simpler but lacks fine-grained control over continuation lifecycle. Original can precisely track and limit continuation usage.

### 2. Continuation Capture Mechanism

| Aspect | Original OmniLisp | HVM4 OmniLisp |
|--------|-------------------|---------------|
| **Method** | `cont_capture(tag)` walks frame chain, clones frames up to delimiter | Implicit via CPS lambda parameter |
| **Delimiter Matching** | Explicit prompt tag matching during frame walk | Identity continuation `(λv. v)` at reset boundary |
| **Value Migration** | CTRR transmigration (values move out of prompt region) | N/A (no region system) |
| **Clone Behavior** | Each frame cloned with inc_ref on contained Obj pointers | HVM4 may copy lambdas arbitrarily |

**Impact**: Original has explicit control over what's captured and handles region-based memory correctly. HVM4 relies on HVM4's interaction net semantics.

### 3. One-shot vs Multi-shot Continuations

| Aspect | Original OmniLisp | HVM4 OmniLisp |
|--------|-------------------|---------------|
| **Default** | One-shot (must opt-in to multi-shot) | **Multi-shot** (HVM4 duplicates lambdas) |
| **Tracking** | `one_shot` boolean + `invoked` flag on Continuation struct | No tracking needed |
| **Enforcement** | `cont_invoke()` checks and returns error if one-shot used twice | N/A - all continuations are multi-shot |
| **Recovery Modes** | Effect types specify: ONE_SHOT, MULTI_SHOT, TAIL_RESUMPTIVE, ABORT | Implicit multi-shot |

**Analysis**: HVM4's approach is actually **more powerful** by default:
- Multi-shot allows `(+ (resume 1) (resume 2))` - continuation used twice
- HVM4 automatically duplicates the lambda when used non-linearly
- One-shot is a *restriction* (optimization), not a requirement

**If one-shot is needed**: Could wrap continuation in a cell that tracks invocation:
```
#Kont1{invoked, k}  ; invoked is 0 or 1
```

**Impact**: Different default, but HVM4's multi-shot default is strictly more expressive.

### 4. Effect System

| Aspect | Original OmniLisp | HVM4 OmniLisp |
|--------|-------------------|---------------|
| **Effect Types** | Registered with: name, unique ID, payload_type, RecoveryProtocol | Just symbol tags (nick-encoded) |
| **Recovery Protocol** | Typed contract: mode, input_type, output_type, description | None |
| **Built-in Effects** | FAIL, ASK, EMIT, STATE, YIELD, ASYNC, CHOICE, CONDITION | None predefined |
| **Resumption** | Struct with: continuation, effect_type, mode, refcount, original_effect | Just `#Kont{k}` |
| **Handler Search** | Walk handler stack, match by effect type ID | Walk list, match by symbol |

**Impact**: Original's effect system is much richer. HVM4 lacks:
- Type safety on effect payloads
- Recovery mode enforcement
- Built-in common effects

### 5. Fiber/Scheduler Implementation

| Aspect | Original OmniLisp | HVM4 OmniLisp |
|--------|-------------------|---------------|
| **Fiber State** | Full CEK state: expr, env, cont (frame chain), value, is_error | `#Fibr{state, cont, mailbox}` where cont is thunk or `#Kont` |
| **States** | READY, RUNNING, PARKED, DONE | #FbrR, #FbrS, #FbrD |
| **Scheduler** | Built-in scheduler with run queue, frame pool, statistics | **Caller-controlled** - no built-in scheduler |
| **Execution Model** | Preemptive with internal scheduler | Cooperative with explicit yield/resume |
| **Context Switch** | Implicit via scheduler | Explicit via `fiber-run`/`fiber-resume` returns |

**Analysis**: The HVM4 approach is actually correct for cooperative fibers:
- `fiber-run` executes until yield, returns new fiber state
- `fiber-resume` invokes captured continuation, returns new fiber state
- Caller decides which fiber to run next (caller IS the scheduler)
- HVM4 evaluates sequentially by default, only parallelizing independent computations

**Tradeoffs**:
- Original: Built-in scheduler is convenient but adds complexity
- HVM4: Pure functional approach - fiber state threaded through, no global scheduler state

### 6. Work-Stealing Thread Pool

| Aspect | Original OmniLisp | HVM4 OmniLisp |
|--------|-------------------|---------------|
| **Architecture** | Chase-Lev work-stealing deques per worker | Basic FFI thread pool |
| **Work Distribution** | Local queue (fast) + global queue + stealing | Global queue only |
| **Worker States** | IDLE, RUNNING, STEALING, SHUTDOWN | No worker concept |
| **Statistics** | tasks_executed, tasks_stolen, steal_attempts, total_steals | None |

**Impact**: Original has much better parallel fiber scheduling. HVM4 thread pool is for FFI calls only.

### 7. Channels (CSP)

| Aspect | Original OmniLisp | HVM4 OmniLisp |
|--------|-------------------|---------------|
| **Implementation** | `FiberChannel` struct with buffer, waiters, select support | **Not yet implemented** |
| **Buffering** | Configurable capacity (0 = unbuffered rendezvous) | Could be pure data structure |
| **Blocking** | Send/recv park the fiber, add to waiter list | Would return `#blocked` + new channel state |
| **Select** | Multi-channel select with `SelectCase` array | Could be implemented as coordinator function |

**Implementation path for HVM4**: Channels can be pure data:
```
#Chan{buffer, recv-waiters, send-waiters}

; send returns (status, new-channel)
; recv returns (value-or-blocked, new-channel)
; Coordinator threads channel state through fiber operations
```

**Impact**: Missing but implementable without scheduler - would use same caller-controlled pattern as fibers.

### 8. Generators/Iterators

| Aspect | Original OmniLisp | HVM4 OmniLisp |
|--------|-------------------|---------------|
| **Structure** | `Generator` struct with state, continuation, current value, producer | **Not implemented** (could use fiber + yield) |
| **States** | CREATED, SUSPENDED, RUNNING, DONE | N/A |
| **Yield** | Captures continuation via `cont_capture()`, stores in generator | Could use `#Yld` but no generator wrapper |
| **API** | `generator_next()`, `generator_is_done()`, `generator_close()` | N/A |

**Impact**: No first-class generator support. Users would need to manually manage fiber state.

### 9. Promises/Async

| Aspect | Original OmniLisp | HVM4 OmniLisp |
|--------|-------------------|---------------|
| **Structure** | `Promise` struct with state, value, error, waiters, callbacks | **Not implemented** |
| **States** | PENDING, FULFILLED, REJECTED | N/A |
| **Await** | Parks fiber on waiter list, unparks on resolve/reject | N/A |
| **Then** | Callback lists for fulfill/reject | N/A |
| **Timer Integration** | `timer_after()`, `await_timeout()` | N/A |

**Impact**: No async/await pattern support. Cannot integrate with external event sources.

### 10. CEK Machine vs Direct Evaluation

| Aspect | Original OmniLisp | HVM4 OmniLisp |
|--------|-------------------|---------------|
| **Design** | Unified CEK machine with explicit frames for all evaluation | Direct recursive + on-demand CPS |
| **Trampoline** | `cek_trampoline()` unified with CEK frames | Not needed (HVM4 handles stack) |
| **Step Function** | `cek_step()` returns DONE, CONTINUE, YIELD, ERROR | No step-based execution |
| **Thunks** | Represented as `FRAME_APP_DONE` frames | Not applicable |

**Impact**: Original can step through evaluation, implement debuggers, handle yields uniformly. HVM4 relies on HVM4's evaluation model.

### 11. Region/Memory Integration (CTRR)

| Aspect | Original OmniLisp | HVM4 OmniLisp |
|--------|-------------------|---------------|
| **Prompt Regions** | Each prompt creates child region; region exited on prompt exit | N/A |
| **Transmigration** | Values captured in continuation are transmigrated to parent region | N/A |
| **Region Stack** | Thread-local stack of active regions | N/A |
| **Lifetime Tracking** | `lifetime_rank` for outlives relationships | N/A |

**Impact**: Original has deterministic memory behavior around continuations. HVM4 relies entirely on interaction net GC.

---

## Recommendations

### Architectural Wins of HVM4 Approach

1. **Fibers work correctly** - caller-controlled scheduling is pure functional
2. **Multi-shot by default** - more powerful than one-shot restriction
3. **No global scheduler state** - fiber state threaded through explicitly
4. **HVM4 SUP for parallelism** - automatic parallelization of independent work

### Suggested Additions

1. **Channels** - optional, defer decision (fibers + effects may suffice)
2. **Generator wrapper** - thin layer on fiber + yield
3. **Effect type registry** - optional, for type safety on payloads
4. **One-shot wrapper** - if needed for specific use cases

### Not Needed

- Built-in scheduler (caller controls scheduling)
- Work-stealing (HVM4 handles parallelism via SUP)
- Region integration (HVM4 GC handles memory)
- CEK machine (HVM4's evaluation model suffices)

---

## Summary: Different Philosophy, Not Missing Features

| Aspect | Original | HVM4 | Assessment |
|--------|----------|------|------------|
| Fiber scheduling | Built-in scheduler | Caller-controlled | ✅ Both valid |
| Continuations | One-shot default | Multi-shot default | ✅ HVM4 more powerful |
| Parallelism | Work-stealing pool | HVM4 SUP | ✅ HVM4 native |
| Memory | Region-based CTRR | Interaction net GC | ✅ Different but correct |
| Channels | Built-in with parking | Not yet implemented | ℹ️ Optional (defer) |
| Effect types | Typed registry | Symbol matching | ⚠️ Optional enhancement |

The HVM4 implementation takes a **pure functional approach** where state (fibers, channels) is threaded through explicitly rather than managed by global schedulers. This is actually cleaner and more composable.
