# OmniLisp Concurrency Guide

OmniLisp runs on HVM4, which provides **automatic parallelism through superposition**. This guide covers concurrency patterns and best practices.

## Core Principles

### 1. Parallel by Default

HVM4 evaluates independent expressions in parallel automatically:

```lisp
;; These run in PARALLEL (no dependencies)
(let [a (expensive-computation-1)]
     [b (expensive-computation-2)]
  (combine a b))
```

### 2. Superposition-Based Parallelism

HVM4's parallelism is based on **superposition** (quantum-inspired), not threads:
- No shared mutable state
- No locks or synchronization primitives needed
- Deterministic results (same input → same output)

### 3. Effects for Communication

Inter-computation communication uses **effects**, not channels:

```lisp
;; Instead of Go-style channels:
;; (send! ch value)  ← NOT this

;; Use effects:
(perform Send value)  ;; Handled by enclosing handler
```

---

## Parallelism Patterns

### Parallel Map

`map` is parallel by default:

```lisp
;; Processes elements in parallel
(map expensive-fn '(1 2 3 4 5))

;; For sequential (when side effects matter):
(map_seq print-fn items)
```

### Parallel Filter

```lisp
;; Predicates evaluated in parallel
(filter expensive-predicate? large-list)

;; Sequential for side-effectful predicates:
(filter_seq stateful-pred? items)
```

### Parallel Fold (Tree Reduction)

For associative operations, use tree reduction:

```lisp
;; Sequential (left-to-right accumulator):
(foldl + 0 '(1 2 3 4))  ;; ((((0+1)+2)+3)+4)

;; Parallel (tree structure):
(reduce_tree + '(1 2 3 4))  ;; (1+2) || (3+4) → 3+7 → 10
```

### Fork/Choice (Superposition)

Create multiple computation branches:

```lisp
;; Both branches computed in parallel
(fork2 computation-a computation-b)

;; Non-deterministic choice
(choice '(option-1 option-2 option-3))

;; Race: first successful result wins
(race risky-fast-path safe-slow-path)
```

---

## Effect-Based Concurrency

### State Effect

Scoped mutable state within handlers:

```lisp
(handle
  (do
    (perform Put 42)
    (+ (perform Get nil) 10))
  (Get [_ resume] (resume current-state))
  (Put [v resume] (resume nil)))  ;; → 52
```

### Exploration Effect

Non-deterministic exploration with backtracking:

```lisp
(handle
  (let [x (perform Explore '(1 2 3))]
    (let [y (perform Explore '(a b))]
      (list x y)))
  (Explore [options resume]
    (fork-all (map resume options))))
;; Produces: ((1 a) (1 b) (2 a) (2 b) (3 a) (3 b))
```

### Memoization Effect

Cache expensive computations:

```lisp
(define memo-expensive [x]
  (match (perform MemoLookup x)
    (Some v) v
    nothing  (let [result (expensive x)]
               (do (perform MemoStore (list x result))
                   result))))

(handle
  (memo-expensive 42)
  (MemoLookup [key resume] (resume (cache-get key)))
  (MemoStore [kv resume] (do (cache-set! kv) (resume nil))))
```

### Resource Management Effect

Safe acquire/release with automatic cleanup:

```lisp
(define with-resource [type body]
  (let [h (perform Acquire type)]
    (let [result (body h)]
      (do (perform Release h) result))))

(handle
  (with-resource 'database
    (lambda [db] (query db "SELECT *")))
  (Acquire [type resume] (resume (open-connection type)))
  (Release [h resume] (do (close-connection h) (resume nil))))
```

---

## Fibers (Cooperative Concurrency)

Lightweight coroutines via CPS transformation:

```lisp
;; Spawn a fiber
(fiber-spawn (lambda [] (do (fiber-yield) "done")))

;; Yield control
(fiber-yield)

;; Resume fiber
(fiber-resume fib)
```

### Fiber Communication via Effects

```lisp
;; Producer fiber
(define producer []
  (do
    (perform Emit 1)
    (fiber-yield)
    (perform Emit 2)
    (fiber-yield)
    (perform Emit 3)))

;; Consumer handles emissions
(handle
  (producer)
  (Emit [value resume]
    (do (process value) (resume nil))))
```

---

## Parallel Let Bindings

Independent bindings evaluate in parallel:

```lisp
;; a and b computed in parallel
(let [a (slow-computation-1)]
     [b (slow-computation-2)]
  (+ a b))

;; Sequential when dependencies exist
(let [a (compute-base)]
  (let [b (compute-from a)]  ;; b depends on a
    (+ a b)))
```

### Dependency Metadata (Future)

```lisp
;; Explicit dependency hints
(let ^:parallel
     [a (compute-1)]
     [b (compute-2)]
     [c ^:after(a) (compute-from a)]  ;; c waits for a
  (combine a b c))
```

---

## Speculative Execution

Execute branches speculatively, discard unused:

```lisp
;; Both branches may start, only one result used
(speculate-if condition
  then-branch
  else-branch)

;; Pattern match with speculative arms
(speculate-match value
  pattern1 result1
  pattern2 result2)
```

**Requirements:**
- Branches must be pure (no side effects)
- HVM4 runtime handles branch cancellation

---

## Speculative Transactions

Speculative transactions combine effects with superposition for racing multiple strategies with automatic rollback.

### Core Primitives

| Primitive | Purpose | Example |
|-----------|---------|---------|
| `commit` | Succeed with value | `(commit 42)` → 42 |
| `rollback` | Abort transaction | `(rollback)` → nothing |
| `speculative-transaction` | Race strategies | First commit wins |
| `with-rollback` | Execute with cleanup | Cleanup on failure |

### Basic Usage

```lisp
;; Commit succeeds with value
(commit 42)  ;; -> 42

;; Rollback aborts (returns nothing)
(rollback)             ;; -> nothing
(rollback "reason")    ;; -> nothing (with diagnostic)

;; Race multiple strategies - first to commit wins
(speculative-transaction
  (commit 10)    ;; <- wins (first)
  (commit 20)
  (commit 30))
;; -> 10
```

### Racing Optimization Strategies

```lisp
;; Try multiple optimization algorithms in parallel
(speculative-transaction
  (let [result (optimize-greedy data)]
    (if (valid? result) (commit result) (rollback)))
  (let [result (optimize-dynamic data)]
    (if (valid? result) (commit result) (rollback)))
  (let [result (optimize-genetic data)]
    (if (valid? result) (commit result) (rollback))))
;; First strategy to find valid solution wins
```

### Conditional Commit/Rollback

```lisp
;; Commit only if validation passes
(speculative-transaction
  (let [x (compute-option-a)]
    (if (> x threshold)
        (commit x)
        (rollback "below threshold")))
  (let [y (compute-option-b)]
    (if (> y threshold)
        (commit y)
        (rollback "below threshold"))))
```

### Cleanup with `with-rollback`

```lisp
;; Execute cleanup on rollback
(with-rollback
  (let [resource (acquire-resource)]
    (let [result (process resource)]
      (if (valid? result)
          (commit result)
          (rollback))))
  (release-resource))  ;; Runs if rollback occurs
```

### Nested Transactions

```lisp
;; Inner transaction within outer
(speculative-transaction
  (rollback)  ;; First option fails
  (speculative-transaction
    (rollback)    ;; Nested first fails
    (commit 25))) ;; Nested second succeeds
;; -> 25
```

### HVM4 Parallel Execution

In HVM4, `speculative-transaction` creates **superpositions**:
- All strategies execute in parallel
- First successful commit "collapses" the superposition
- Failed branches are automatically discarded
- No manual synchronization needed

```lisp
;; These run in PARALLEL on HVM4:
(speculative-transaction
  expensive-strategy-a
  expensive-strategy-b
  expensive-strategy-c)
;; Fastest successful strategy wins
```

### Use Cases

| Pattern | Description |
|---------|-------------|
| Racing algorithms | Try multiple approaches, use first success |
| Validation with fallback | Validate, rollback if invalid |
| Speculative search | Explore branches, commit on match |
| Resource allocation | Try acquire, rollback on failure |
| Optimistic concurrency | Attempt operation, rollback on conflict |

---

## Purity Annotations

The `^:pure` metadata enables the compiler to verify and optimize parallel execution.

### Declaring Pure Functions

```lisp
;; Mark a function as pure
(define ^:pure square [x]
  (* x x))

;; Pure function with multiple arguments (curried)
(define ^:pure add [x] [y]
  (+ x y))

;; Pure function with let bindings
(define ^:pure calculate [a] [b]
  (let [sum (+ a b)]
       [diff (- a b)]
    (* sum diff)))
```

### What Pure Means

A function is **pure** if it:
1. **No side effects** — Doesn't print, mutate globals, perform I/O
2. **Referentially transparent** — Same inputs always produce same outputs
3. **No effects** — Doesn't use `perform` (except exploration)
4. **Terminates** — Doesn't loop infinitely (ideally)

```lisp
;; Pure: only uses inputs, returns computed value
(define ^:pure triple [x] (* x 3))

;; NOT pure: prints to console
(define log-triple [x]
  (do (println x) (* x 3)))

;; NOT pure: reads global state
(define config-triple [x]
  (* x (get-config 'multiplier)))
```

### Purity and Parallel Execution

The compiler uses purity to decide parallelization:

```lisp
;; Parallel (square is ^:pure)
(map square xs)

;; Sequential (log-triple has side effects)
(map_seq log-triple xs)

;; Auto-detection: compiler infers purity when possible
(map (lambda [x] (* x x)) xs)  ;; Parallel (lambda body is pure)
```

### Purity Levels

| Level | Metadata | Behavior |
|-------|----------|----------|
| Pure | `^:pure` | Safe for parallel, memoization, CSE |
| Effectful | (default) | May have effects, ordered evaluation |
| Strict | `^:strict` | Forces immediate evaluation |

### Superposition Types (HVM4)

Pure functions can return superpositions:

```lisp
;; Superposition type: multiple possible values
{(Sup Int)}  ;; Type of Int superposition

;; Create superposition via exploration
(let [x (explore '(1 2 3))]
  (* x x))
;; Type: {(Sup Int)} — superposition of 1, 4, 9

;; Collapse superposition to list
(collapse (map square (explore '(1 2 3))))
;; -> '(1 4 9)
```

### Purity Guarantees

| Guarantee | Enabled By |
|-----------|------------|
| Parallel map/filter | `^:pure` on function |
| Memoization | `^:pure` (same input = same output) |
| Common subexpression elimination | `^:pure` |
| Speculative execution | `^:pure` on branches |
| Automatic parallelization | `^:pure` + dependency analysis |

---

## Staged Parallelism (Advanced)

Staged parallelism uses multi-stage programming (the tower of interpreters) to generate specialized parallel code at compile time.

### Concept

```lisp
;; At stage 1 (compile time), analyze and generate parallel code
(define compile-parallel-map [f]
  (stage 1
    (if (staged-pure? (reflect f))
        ;; Generate tree-structured parallel code
        `(lambda [xs]
           (reduce-tree
             (lambda [a b] (append a b))
             (map-chunks ,(splice f) xs)))
        ;; Generate sequential code
        `(lambda [xs] (map-seq ,(splice f) xs)))))
```

### Compile-Time Purity Analysis

```lisp
;; staged-pure? checks purity at compile time
(stage 1
  (if (staged-pure? (reflect my-func))
      (generate-parallel-version)
      (generate-sequential-version)))
```

### Adaptive Specialization

```lisp
;; Generate code specialized for input characteristics
(define adaptive-sort
  (stage 1
    `(lambda [xs]
       (if (< (length xs) 1000)
           (insertion-sort xs)        ;; Small: O(n²) but cache-friendly
           (parallel-merge-sort xs))) ;; Large: O(n log n) parallel
    ))

;; Usage - generates optimized code at compile time
(define fast-square-all
  (compile-parallel-map (lambda [x] (* x x))))
```

### Chunked Processing

```lisp
;; Process large data in parallel chunks
(map-chunks f xs chunk-size)
;; Splits xs into chunks, maps f over each chunk in parallel
;; Then combines results

;; Example: parallel image processing
(define parallel-blur [pixels]
  (let [chunks (partition-into pixels 1000)]
    (reduce concat (fork-all (map blur-chunk chunks)))))
```

### Tower Integration

The tower of interpreters enables introspection for parallelization decisions:

```lisp
;; Reflect code to analyze structure
(stage 1
  (match (reflect body)
    ;; Recognize parallelizable patterns
    [(map f xs)]
      (if (staged-pure? f)
          (generate-parallel-map f xs)
          (generate-seq-map f xs))
    ;; Recognize fold patterns
    [(foldl f init xs)]
      (if (and (staged-pure? f) (associative? f))
          (generate-tree-reduce f init xs)
          (generate-seq-fold f init xs))
    ;; Default: preserve original
    other
      other))
```

**Note:** Staged parallelism is an advanced feature primarily useful for library authors and performance-critical code.

---

## Best Practices

### 1. Prefer Pure Functions

Pure functions can safely run in parallel:

```lisp
;; Good: Pure transformation
(map (lambda [x] (* x x)) numbers)

;; Bad: Side-effectful (use sequential)
(map_seq (lambda [x] (do (print x) x)) numbers)
```

### 2. Use Effects for State

Instead of mutable variables, use effect handlers:

```lisp
;; Bad: Global mutable state
(set! counter (+ counter 1))

;; Good: Scoped state effect
(perform Increment nil)
```

### 3. Tree Reduction for Associativity

For associative operations, tree structure enables parallelism:

```lisp
;; String concatenation
(reduce_tree string-concat strings)

;; Sum with parallel reduction
(reduce_tree + numbers)
```

### 4. Scope Effects Tightly

Minimize handler scope for better parallelism:

```lisp
;; Good: Tight scope
(let [result (handle body (Eff [x r] ...))]
  (parallel-use result))

;; Less good: Wide scope
(handle
  (do body1 body2 body3)  ;; All constrained by handler
  (Eff [x r] ...))
```

---

## Performance Considerations

### When Parallel Helps

- Large data transformations
- Independent computations
- Tree-structured algorithms
- Search/exploration problems

### When Sequential is Better

- Small data (parallelism overhead)
- Inherently sequential algorithms
- Side-effectful operations
- Short-circuit evaluation needed

### Profiling Parallelism

```lisp
;; Time parallel vs sequential
(timed (map f large-list))
(timed (map_seq f large-list))

;; Benchmark with warmup
(bench 100 (lambda [] (map f data)))
```

### Debugging Parallel Execution

#### Dependency DAG Visualization

Visualize how parallel let bindings are scheduled:

```lisp
;; Show dependency graph for parallel let
(show-parallel-dag
  '(let ^:parallel
     [a (compute-a)]
     [b (compute-b)]
     [c (compute-c a)]
     [d (combine b c)]
     d))

;; Output (ASCII):
;;   a ────► c ─┬─► d
;;   b ────────┘
;;
;; Stages: [1: {a, b}] [2: {c}] [3: {d}]
```

#### Parallel Execution Tracing

```lisp
;; Trace parallel execution
(trace-parallel
  (let ^:parallel
    [x (expensive-1)]
    [y (expensive-2)]
    [z (combine x y)]
    z))

;; Output:
;; [stage 1] starting: x, y
;; [stage 1] completed: x (42ms)
;; [stage 1] completed: y (38ms)
;; [stage 2] starting: z
;; [stage 2] completed: z (5ms)
;; Total: 47ms (parallel), would be 85ms sequential
```

#### Dependency Analysis

```lisp
;; Analyze dependencies without executing
(analyze-dependencies
  '(let [a 1]
        [b (+ a 1)]
        [c 2]
        [d (+ b c)]
     d))

;; Returns:
;; #{:bindings #{a b c d}
;;   :dependencies #{(b depends-on a) (d depends-on b c)}
;;   :parallel-groups ((a c) (b) (d))
;;   :critical-path (a -> b -> d)}
```

#### Detecting Sequential Bottlenecks

```lisp
;; Identify operations that force sequential execution
(find-sequential-bottlenecks my-function)

;; Returns:
;; #{:impure-calls (println at line 42)
;;   :data-dependencies (fold at line 55 - accumulator threading)
;;   :suggestions ("Consider reduce_tree for parallel reduction")}
```

---

## Probabilistic Programming

OmniLisp supports probabilistic programming via effects and HVM4 superposition.

### Distributions

```lisp
;; Distribution constructors
(bernoulli 75)       ;; Bernoulli with 75% probability
(uniform 0 100)      ;; Uniform distribution [0, 100]
(beta 2 2)           ;; Beta distribution (α=2, β=2)
(categorical '((a 30) (b 70)))  ;; Categorical distribution
```

### Sampling Primitives

```lisp
;; Sample from distribution
(sample (bernoulli 75))  ;; -> true (75% chance) or false

;; Flip is shorthand for sample bernoulli
(flip 50)                ;; -> true or false (50/50)

;; Sample from uniform
(sample (uniform 0 100)) ;; -> value in [0, 100]

;; Sample from beta (returns expected value percentage)
(sample (beta 2 2))      ;; -> 50 (E[Beta(2,2)] = 0.5)
```

### Conditioning

```lisp
;; Observe conditions the model
(observe (= x expected-value))
;; In HVM4: rejects execution paths where condition is false

;; Factor weights execution path
(factor weight)
;; In HVM4: multiplies path probability by weight
```

### Probabilistic Model Example

```lisp
;; Bayesian coin flip model
(define coin-model []
  (let [bias (sample (beta 2 2))]    ;; Prior: uniform-ish
    (do
      (observe (flip bias))           ;; Observed: heads
      (observe (flip bias))           ;; Observed: heads
      (observe (not (flip bias)))     ;; Observed: tails
      bias)))                         ;; Return posterior bias
```

### Inference (HVM4)

In HVM4, probabilistic programs create superpositions that explore all execution paths:

```lisp
;; Enumerate all outcomes with probabilities
(enumerate model)
;; Returns distribution over model outputs

;; Importance sampling (approximate)
(importance-sample model 1000)
;; Returns sampled distribution
```

### C Interpreter Behavior

In the C interpreter, probabilistic primitives are deterministic:
- `(flip p)` returns `true` if p >= 50, `false` otherwise
- `(sample (uniform lo hi))` returns midpoint
- `(sample (beta a b))` returns expected value as percentage

This enables testing probabilistic models without full inference.

---

## Summary

| Pattern | Use Case | Example |
|---------|----------|---------|
| Parallel map | Transform collections | `(map f items)` |
| Tree reduction | Associative aggregation | `(reduce_tree + nums)` |
| Fork/choice | Explore alternatives | `(fork2 a b)` |
| Effects | Communication/state | `(perform Eff payload)` |
| Fibers | Cooperative tasks | `(fiber-spawn fn)` |
| Speculation | Branch prediction | `(speculate-if cond t f)` |

OmniLisp's parallel-first design means you get parallelism by default. Use `_seq` variants and effects when you need ordering guarantees.
