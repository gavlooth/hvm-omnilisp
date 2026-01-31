# Algebraic Effects and Handlers Guide

OmniLisp uses **algebraic effects** instead of traditional exceptions. Effects provide more control, composability, and safety for non-local control flow.

## Why Effects Over Exceptions?

| Feature | try/catch | Algebraic Effects |
|---------|-----------|-------------------|
| Resume after error | No | Yes (via `resume`) |
| Multiple handlers | Catch-all only | Named, composable |
| Testability | Mock entire system | Inject handlers |
| Control flow | One-shot | Resumable, multi-shot |

Effects let you "throw" to a handler, and the handler can **continue** execution from where you left off.

## Basic Concepts

### The Three Primitives

1. **`perform`** - Trigger an effect (like "throw")
2. **`handle`** - Establish handlers (like "try")
3. **`resume`** - Continue from where `perform` was called

```lisp
(handle
  body-expression
  (effect-name [payload resume]
    handler-body))
```

## Your First Effect

### Simple Example

```lisp
(handle
  (+ 1 (perform ask nothing))
  (ask [_ resume]
    (resume 42)))
;; -> 43
```

What happens:
1. Start evaluating `(+ 1 ...)`
2. Hit `(perform ask nothing)` - execution suspends
3. Handler `ask` receives `nothing` and the continuation `resume`
4. Handler calls `(resume 42)` - execution continues with 42
5. Result: `(+ 1 42)` = 43

### Without Resume

If you don't call `resume`, execution stops at the handler:

```lisp
(handle
  (+ 1 (perform fail "error!"))
  (fail [msg _]
    (str "Failed: " msg)))
;; -> "Failed: error!"
```

## Common Effect Patterns

### Error Handling

Replace try/catch with effects:

```lisp
;; Traditional error handling (other languages)
;; try {
;;   return sqrt(x)
;; } catch (NegativeError e) {
;;   return 0
;; }

;; OmniLisp with effects
(define safe-sqrt [x]
  (handle
    (if (< x 0)
        (perform fail "negative number")
        (sqrt x))
    (fail [msg _]
      (println (str "Error: " msg))
      0)))

(safe-sqrt 16)   ;; -> 4
(safe-sqrt -1)   ;; prints "Error: negative number", returns 0
```

### Optional Values

```lisp
;; Handle missing values without Nothing checks everywhere
(define find-or-default [items pred default]
  (handle
    (perform found (find pred items))
    (found [result resume]
      (match result
        [Nothing]  default
        [(Some v)] v))))
```

### Dependency Injection

```lisp
;; Abstract over database operations
(define get-user [id]
  (perform db-query (str "SELECT * FROM users WHERE id = " id)))

;; In production
(handle
  (get-user 42)
  (db-query [sql resume]
    (resume (real-database-query sql))))

;; In tests
(handle
  (get-user 42)
  (db-query [sql resume]
    (resume #{"id" 42 "name" "Test User"})))
```

### Logging

```lisp
(define process-data [data]
  (do
    (perform log "Starting processing")
    (let [result (transform data)]
      (perform log (str "Completed: " result))
      result)))

;; With actual logging
(handle
  (process-data input)
  (log [msg resume]
    (println (str "[LOG] " msg))
    (resume nothing)))

;; Silent (no logging)
(handle
  (process-data input)
  (log [_ resume]
    (resume nothing)))

;; Collect logs
(handle
  (let [logs (ref '())]
    (handle
      (process-data input)
      (log [msg resume]
        (set! logs (cons msg @logs))
        (resume nothing)))
    (reverse @logs)))
```

## Multiple Effects

Handle multiple effect types in one block:

```lisp
(handle
  (let [data (perform read-file "config.json")]
    (let [config (perform parse-json data)]
      (perform log (str "Loaded config: " config))
      config))

  (read-file [path resume]
    (resume (slurp path)))

  (parse-json [str resume]
    (resume (json-parse str)))

  (log [msg resume]
    (println msg)
    (resume nothing))

  (error [msg _]
    (println (str "Error: " msg))
    #{}))
```

## Effect Propagation

Effects bubble up to the nearest handler:

```lisp
(define inner []
  (perform ask nothing))

(define middle []
  (+ 1 (inner)))

(define outer []
  (handle
    (middle)
    (ask [_ resume]
      (resume 10))))

(outer)  ;; -> 11
```

The `ask` effect in `inner` propagates through `middle` to the handler in `outer`.

## Resumable vs Non-Resumable

### Resumable (One-Shot)

Most effects are one-shot: call `resume` once or not at all.

```lisp
(handle
  (perform get-input nothing)
  (get-input [_ resume]
    (resume (read-line))))
```

### Multi-Shot (Advanced)

Some effects can resume multiple times (for non-determinism):

```lisp
;; Choose from multiple options
(define ^:multi-shot choice [options]
  (perform choose options))

(handle
  (let [x (choice '(1 2 3))]
    (let [y (choice '(10 20))]
      (list x y)))
  (choose [options resume]
    (map resume options)))
;; -> '((1 10) (1 20) (2 10) (2 20) (3 10) (3 20))
```

## Defining Effect Types

### Declare Effects

```lisp
;; Declare an effect type (optional but recommended)
(define ^:effect ask)
(define ^:effect tell)
(define ^:effect fail)
```

### One-Shot Effects

```lisp
;; Most effects are one-shot by default
(define ^:one-shot {effect fail})
```

### Effect Signatures

```lisp
;; Effect with typed payload and return
(define ^:effect read-file
  {fn [{String}] {String}})

(define ^:effect parse
  {fn [{String}] {Any}})
```

## Common Effect Library

### State Effect

```lisp
;; Encapsulate mutable state
(define with-state [initial body]
  (let [state (ref initial)]
    (handle
      (body)
      (get-state [_ resume]
        (resume @state))
      (set-state [new-val resume]
        (set! state new-val)
        (resume nothing)))))

;; Usage
(with-state 0
  (lambda []
    (perform set-state 10)
    (let [x (perform get-state nothing)]
      (perform set-state (+ x 5))
      (perform get-state nothing))))
;; -> 15
```

### Reader Effect

```lisp
;; Read-only environment
(define with-env [env body]
  (handle
    (body)
    (ask-env [key resume]
      (resume (get env key)))))

;; Usage
(with-env #{"db-host" "localhost" "db-port" 5432}
  (lambda []
    (let [host (perform ask-env "db-host")]
      (let [port (perform ask-env "db-port")]
        (str host ":" port)))))
;; -> "localhost:5432"
```

### Writer Effect

```lisp
;; Accumulate output
(define with-writer [body]
  (let [output (ref '())]
    (handle
      (let [result (body)]
        (list result (reverse @output)))
      (tell [msg resume]
        (set! output (cons msg @output))
        (resume nothing)))))

;; Usage
(with-writer
  (lambda []
    (perform tell "step 1")
    (perform tell "step 2")
    42))
;; -> (42 ("step 1" "step 2"))
```

### Exception Effect

```lisp
;; Traditional exception semantics
(define with-exceptions [body handlers]
  (handle
    (body)
    (throw [exn _]
      (let [handler (get handlers (exception-type exn))]
        (if handler
            (handler (exception-payload exn))
            (perform throw exn))))))  ;; Re-raise if unhandled

;; Usage
(with-exceptions
  (lambda []
    (if (< x 0)
        (perform throw (make-exception 'ValueError "negative"))
        (sqrt x)))
  #{'ValueError (lambda [msg] 0)})
```

## Effect Composition

### Layering Handlers

Handlers compose naturally:

```lisp
;; Outer handler for logging
(handle
  ;; Inner handler for errors
  (handle
    (let [x (perform read-input nothing)]
      (perform log (str "Got: " x))
      (if (= x "quit")
          (perform fail "User quit")
          (process x)))
    (fail [msg _]
      (perform log (str "Failed: " msg))
      "error"))
  (read-input [_ resume]
    (resume (read-line)))
  (log [msg resume]
    (println msg)
    (resume nothing)))
```

### Effect Transformers

```lisp
;; Add logging to any computation
(define with-logging [body]
  (let [call-count (ref 0)]
    (handle
      (body)
      (any-effect [payload resume]
        (set! call-count (+ 1 @call-count))
        (println (str "Effect #" @call-count ": " payload))
        ;; Re-raise to let outer handler process
        (resume (perform any-effect payload))))))
```

## Exploration Effect (Non-Deterministic Search)

The exploration effect enables non-deterministic search with automatic backtracking. In HVM4, this leverages **superposition** to explore multiple paths in parallel.

### Core Primitives

| Primitive | Purpose | Example |
|-----------|---------|---------|
| `explore` | Choose from multiple options | `(explore '(1 2 3))` |
| `choice` | Alias for explore | `(choice '(a b c))` |
| `reject` | Abandon current path (fail) | `(reject)` |
| `require` | Continue only if condition holds | `(require (> x 0))` |

### Basic Exploration

```lisp
;; Choose from a list of options
(let [x (explore '(1 2 3))]
  (* x x))
;; In HVM4: explores ALL branches in parallel via superposition
;; Result depends on handler: all values, first, or filtered

;; Reject invalid paths
(let [x (explore '(1 2 3 4 5))]
  (do
    (require (= 0 (% x 2)))  ;; Only even numbers
    x))
;; Explores: 1→reject, 2→accept, 3→reject, 4→accept, 5→reject
;; Results: 2, 4
```

### Exploration Helpers

```lisp
;; Find first value satisfying predicate
(explore-first '(1 2 3 4 5) (lambda [x] (> x 3)))
;; -> 4

;; Collect all valid results
(explore-all '(1 2 3 4 5 6)
  (lambda [x]
    (if (= 0 (% x 2)) (* x 2) nothing)))
;; -> '(4 8 12)  ;; doubled evens

;; Explore numeric range
(explore-range 0 10)  ;; Choose from 0..9
```

### Effect-Based Exploration Handler

```lisp
;; Manual exploration with effect handler
(handle
  (let [x (perform Explore '(1 2 3))]
    (let [y (perform Explore '(a b))]
      (list x y)))
  (Explore [options resume]
    ;; Multi-shot: resume for EACH option
    (map resume options)))
;; -> '((1 a) (1 b) (2 a) (2 b) (3 a) (3 b))
```

### Constraint Solving Pattern

```lisp
;; Find Pythagorean triples: a² + b² = c²
(define find-pythagorean [max]
  (let [a (explore-range 1 max)]
    (let [b (explore-range a max)]      ;; b >= a
      (let [c (explore-range b max)]    ;; c >= b
        (do
          (require (= (+ (* a a) (* b b)) (* c c)))
          (list a b c))))))

;; In HVM4, all valid (a,b,c) combinations explored in parallel
```

### Backtracking Search Pattern

```lisp
;; N-Queens problem: place n queens on n×n board
(define n-queens [n]
  (define safe? [queens col row]
    (let [check-queen [i q]
      (let [diag-dist (- col i)]
        (and (not (= q row))
             (not (= q (+ row diag-dist)))
             (not (= q (- row diag-dist)))))]
      (every check-queen (enumerate queens))))

  (define place [col queens]
    (if (= col n)
        queens
        (let [row (explore-range 0 n)]
          (do
            (require (safe? queens col row))
            (place (+ col 1) (append queens (list row)))))))

  (place 0 '()))
```

### Parallel Exploration (HVM4)

In HVM4, exploration creates **superpositions**—multiple computation branches that execute in parallel:

```lisp
;; These run in PARALLEL on HVM4:
(let [x (explore '(1 2 3))]
  (expensive-computation x))
;; Creates 3 parallel branches, each computing independently

;; Combine with fork2 for explicit parallelism:
(fork2
  (explore-first opts1 pred1)
  (explore-first opts2 pred2))
```

### Collect vs First Semantics

| Function | Behavior | Use Case |
|----------|----------|----------|
| `explore-first` | Return first valid result | Search with early exit |
| `explore-all` | Collect all valid results | Enumerate solutions |
| `explore` (raw) | Handler determines | Custom exploration |

```lisp
;; First match
(explore-first '(1 2 3 4) even?)  ;; -> 2

;; All matches
(explore-all '(1 2 3 4) (lambda [x] (if (even? x) x nothing)))
;; -> '(2 4)
```

### Composing Exploration with Other Effects

```lisp
;; Exploration + State (counting branches explored)
(handle
  (handle
    (let [x (perform Explore '(1 2 3))]
      (do
        (perform Increment nothing)
        (* x x)))
    (Explore [opts resume]
      (map resume opts)))
  (Increment [_ resume]
    (perform Log "Branch explored")
    (resume nothing)))

;; Exploration + Logging
(handle
  (let [x (explore '(a b c))]
    (do
      (perform Log (str "Trying: " x))
      x))
  (Log [msg resume]
    (println msg)
    (resume nothing)))
```

## Effect vs Pattern Matching

Both are powerful; use the right tool:

| Use Case | Pattern Matching | Effects |
|----------|-----------------|---------|
| Structural dispatch | Yes | No |
| Error recovery | Limited | Yes |
| Dependency injection | No | Yes |
| Non-local control | No | Yes |
| Performance critical | Yes | Overhead |

```lisp
;; Pattern matching: local, structural
(match result
  [(Ok v)]  (process v)
  [(Err e)] (handle-error e))

;; Effects: non-local, resumable
(handle
  (do-something-that-might-fail)
  (fail [e resume]
    (resume (recover-from e))))
```

## Best Practices

### 1. Name Effects Descriptively

```lisp
;; Bad
(perform e data)

;; Good
(perform db-query sql)
(perform log-message msg)
(perform validate-input data)
```

### 2. Handle Close to Usage

```lisp
;; Bad: Handler far from effects
(handle
  (deeply-nested-function)
  (all-the-effects ...))

;; Good: Handler wraps specific concern
(define with-database [body]
  (handle
    (body)
    (db-query [sql resume]
      (resume (execute-sql sql)))))
```

### 3. Provide Default Handlers

```lisp
;; Module provides default behavior
(define default-logging-handler
  (lambda [msg resume]
    (println (str "[LOG] " msg))
    (resume nothing)))

;; Users can override
(handle
  (my-computation)
  (log [msg resume]
    (my-custom-logger msg)
    (resume nothing)))
```

### 4. Document Effect Requirements

```lisp
;; Document which effects a function may perform
(define ^:effects [db-query log fail]
  process-user [id]
  (let [user (perform db-query (str "SELECT * FROM users WHERE id = " id))]
    (perform log (str "Processing user: " (get user "name")))
    (if (banned? user)
        (perform fail "User is banned")
        (compute-result user))))
```

## Performance Considerations

1. **Handler lookup is O(n)** where n = number of handlers
2. **Continuation capture has overhead** - don't use for tight loops
3. **Prefer pattern matching** for hot paths
4. **Use effects for boundaries** - I/O, errors, dependencies

```lisp
;; Bad: Effect in tight loop
(define sum-with-logging [xs]
  (foldl (lambda [acc x]
           (perform log (str "Adding " x))
           (+ acc x))
         0 xs))

;; Good: Effect at boundary
(define sum-with-logging [xs]
  (let [result (foldl + 0 xs)]
    (perform log (str "Sum: " result))
    result))
```

## Summary

| Operation | Syntax | Purpose |
|-----------|--------|---------|
| Trigger effect | `(perform name payload)` | Signal to handler |
| Handle effect | `(handle body (name [p r] ...))` | Establish handler |
| Continue | `(resume value)` | Return to `perform` |
| Declare effect | `(define ^:effect name)` | Type-level documentation |

## Effect Rows

Effect rows track what effects a function may perform, enabling automatic parallelization and static analysis.

```lisp
;; Declare effect row on function
(define log-value [x]
  ^:effects [IO]
  (do (println x) x))

;; Pure function (empty effect row)
(define ^:pure add [x] [y] (+ x y))
;; Equivalent to: ^:effects []

;; Check if function is effect-free
(effect-free? add)        ;; -> true
(effect-free? log-value)  ;; -> false
```

See **[Effect Rows Guide](EFFECT_ROWS.md)** for complete documentation on:
- Effect row syntax and inference
- The `effect-free?` predicate
- Smart dispatch based on purity
- Effect row polymorphism (planned)

## Further Reading

- [Quick Reference](QUICK_REFERENCE.md) - Effects syntax overview
- [Effect Rows Guide](EFFECT_ROWS.md) - Effect tracking and purity
- [API Reference](API_REFERENCE.md) - Effect-related functions
- [Pattern Matching Guide](PATTERN_MATCHING_GUIDE.md) - Structural alternatives
- [Type System Guide](TYPE_SYSTEM_GUIDE.md) - Effect typing

## References

- "An Introduction to Algebraic Effects and Handlers" (Pretnar, 2015)
- "Abstracting Control" (Danvy & Filinski, 1990)
- "Effect Handlers in Scope" (Wu, Schrijvers, 2014)
