# Advanced Features Guide

This guide covers OmniLisp's advanced features: the Tower of Interpreters for meta-programming, the macro system, fibers for concurrency, and FFI for native code integration.

## Tower of Interpreters

The Tower of Interpreters is OmniLisp's system for multi-stage programming and reflection. It enables runtime code generation, partial evaluation, and meta-level manipulation.

### Core Concepts

The tower has levels:
- **Level 0**: Normal execution
- **Level 1**: Code manipulation (meta-level)
- **Level 2**: Meta-meta-level
- **...**

You can move up and down the tower to manipulate code at different abstraction levels.

### Code Values

Create code objects without evaluating them:

```lisp
;; Create a code value
(code (+ 1 2))
;; -> #Cod{#Add{#Lit{1}, #Lit{2}}}

;; The code is data, not executed
(define my-code (code (* x y)))
```

### Executing Code

Run code values:

```lisp
;; Execute code
(code-exec (code (+ 1 2)))
;; -> 3

;; With environment
(code-exec-with (code (+ x 10)) #{"x" 5})
;; -> 15
```

### Reflecting Values to Code

Convert runtime values to code:

```lisp
;; Reflect a value
(reflect 42)
;; -> #Cod{#Lit{42}}

;; Useful for code generation
(define make-adder [n]
  (code-exec
    (code (lambda [x] (+ x ~(reflect n))))))

(define add5 (make-adder 5))
(add5 10)  ;; -> 15
```

### Meta-Level Evaluation

Move up the tower:

```lisp
;; EM - evaluate at meta-level
(EM expr)

;; shift - go up n levels
(shift 1 expr)

;; Get current level
(meta-level)  ;; -> 0 at base
```

### Compiled Lambdas

Create optimized functions:

```lisp
;; clambda - compiled lambda
(define fast-square
  (clambda [x] (* x x)))

;; Same semantics, but compiled
(fast-square 5)  ;; -> 25
```

### Partial Evaluation

Optimize code at compile time:

```lisp
;; Partial evaluation optimizes known values
(pe-optimize (code (+ 1 2)))
;; -> #Cod{#Lit{3}}

;; With unknown values
(pe-optimize (code (+ 1 x)))
;; -> #Cod{#Add{#Lit{1}, #Var{x}}}
```

### Staging

Generate specialized code:

```lisp
;; Stage computation
(define power [n]
  (if (= n 0)
      (code 1)
      (code (* x ~(power (- n 1))))))

;; Generate x^5 code
(power 5)
;; -> (code (* x (* x (* x (* x (* x 1))))))

;; Compile specialized function
(define pow5 (code-exec (code (lambda [x] ~(power 5)))))
(pow5 2)  ;; -> 32
```

### Quasiquotation in Tower

Build code with splicing:

```lisp
;; Quasiquote with unquote
`(+ 1 ,(+ 2 3))
;; -> '(+ 1 5)

;; Unquote-splicing
`(+ ,@'(1 2 3))
;; -> '(+ 1 2 3)

;; In code generation
(define make-sum [exprs]
  `(+ ,@exprs))

(make-sum '(a b c))
;; -> '(+ a b c)
```

## Macro System

OmniLisp macros transform code at compile time. They're hygienic and support ellipsis patterns.

### Basic Macros

```lisp
;; Define a macro
(define [syntax when]
  [(when test body ...)
   (match test
     [false nothing]
     [_ (do body ...)])])

;; Usage
(when (> x 0)
  (println "positive")
  (process x))

;; Expands to:
(match (> x 0)
  [false nothing]
  [_ (do (println "positive") (process x))])
```

### Pattern Matching in Macros

```lisp
;; Multiple patterns
(define [syntax unless]
  [(unless test body ...)
   (when (not test) body ...)])

;; With destructuring
(define [syntax let1]
  [(let1 [name value] body ...)
   (let [name value] body ...)])
```

### Ellipsis Patterns

`...` matches zero or more elements:

```lisp
;; Match and reproduce multiple items
(define [syntax list*]
  [(list* items ...)
   (list items ...)])

;; Nested ellipsis
(define [syntax let*]
  [(let* () body)
   body]
  [(let* ([var val] rest ...) body)
   (let [var val] (let* (rest ...) body))])

;; Usage
(let* ([x 1] [y (+ x 1)] [z (+ y 1)])
  z)
;; -> 3
```

### Gensyms for Hygiene

Generate unique symbols to avoid capture:

```lisp
(define [syntax with-temp]
  [(with-temp body ...)
   (let [temp (gensym "temp")]
     `(let [,temp nothing]
        ,@body))])
```

### Hygiene Implementation Details

OmniLisp uses a **mark-based hygiene system** similar to Scheme's `syntax-case`. This prevents accidental variable capture without requiring explicit gensyms for most macros.

#### How It Works

1. **Hygiene Marks (`#HMrk{id}`)**: Each macro expansion creates a unique mark
2. **Hygienic Symbols (`#HSym{nick, marks}`)**: Symbols carry a set of marks showing their expansion context
3. **Comparison**: Two symbols are equal only if their nicks AND marks match

#### Internal Representations

| AST Node | Description | Example |
|----------|-------------|---------|
| `#HMrk{id}` | Unique mark per expansion | `#HMrk{42}` |
| `#HSym{nick, marks}` | Symbol with mark set | `#HSym{x, [#HMrk{42}]}` |

#### Key Functions

```lisp
;; Generate fresh mark for this expansion
(fresh-mark)  ; → #HMrk{unique-id}

;; Add mark to a symbol
(add-mark sym mark)  ; → #HSym{nick, marks ∪ {mark}}

;; Check if symbol is hygienic (has marks)
(hygienic? sym)  ; → true/false

;; Compare symbols hygienically
(sym-eq-hygienic a b)  ; → true only if nicks AND marks match
```

#### Example: Why Marks Prevent Capture

```lisp
;; Macro that introduces 'x'
(define [syntax swap-demo]
  [(swap-demo a b)
   (let [x a]       ;; This 'x' gets marked
     (let [a b]
       (let [b x]   ;; This 'x' refers to marked 'x'
         (list a b))))])

;; User code with their own 'x'
(let [x 100]
  (swap-demo x 200))

;; Without hygiene: x=100 would be captured by macro's x
;; With hygiene: macro's x has mark, user's x doesn't - they're distinct!
;; Result: (200 100) - correct swap behavior
```

#### Expansion Process

1. **Before expansion**: Generate fresh mark for this expansion
2. **Pattern match**: Bind pattern variables to code fragments
3. **Template substitution**:
   - Pattern variables → substitute captured code (no marks added)
   - Introduced symbols → add current mark
4. **Output**: Transformed code with marks on introduced symbols

#### Checking Hygiene Violations

```lisp
;; Verify a macro is hygienic
(hygiene-check 'when)
;; → #True{} if no variable capture possible
;; → #Fals{} with report if capture detected
```

### Recursive Macros

```lisp
;; Thread-first macro
(define [syntax ->]
  [(-> x)
   x]
  [(-> x (f args ...) rest ...)
   (-> (f x args ...) rest ...)]
  [(-> x f rest ...)
   (-> (f x) rest ...)])

;; Usage
(-> 5
    (+ 3)
    (* 2)
    str)
;; Expands to: (str (* (+ 5 3) 2))
;; -> "16"
```

### Common Macro Patterns

#### Short-circuit Evaluation

```lisp
(define [syntax and]
  [(and) true]
  [(and x) x]
  [(and x rest ...)
   (if x (and rest ...) false)])

(define [syntax or]
  [(or) false]
  [(or x) x]
  [(or x rest ...)
   (let [temp x]
     (if temp temp (or rest ...)))])
```

#### Control Flow

```lisp
(define [syntax cond]
  [(cond) nothing]
  [(cond [test result] rest ...)
   (if test result (cond rest ...))])

(define [syntax case]
  [(case expr [val result] ... [else default])
   (let [temp expr]
     (cond
       [(= temp val) result] ...
       [true default]))])
```

#### Binding Forms

```lisp
(define [syntax when-let]
  [(when-let [name expr] body ...)
   (let [temp expr]
     (match temp
       [Nothing] nothing
       [(Some name)] (do body ...)))])
```

## Fibers and Concurrency

OmniLisp provides lightweight fibers for concurrent programming.

### Creating Fibers

```lisp
;; Create a fiber (suspended)
(define fib (fiber (lambda []
  (println "Hello from fiber")
  42)))

;; Resume fiber
(fiber-resume fib)
;; prints "Hello from fiber"
;; -> 42
```

### Yielding

```lisp
;; Fiber that yields
(define counter-fiber (fiber (lambda []
  (yield 1)
  (yield 2)
  (yield 3)
  'done)))

(fiber-resume counter-fiber)  ;; -> 1
(fiber-resume counter-fiber)  ;; -> 2
(fiber-resume counter-fiber)  ;; -> 3
(fiber-resume counter-fiber)  ;; -> 'done
```

### Fiber Scheduler

```lisp
;; Run multiple fibers cooperatively
(with-fibers
  (spawn (fiber (lambda []
    (println "Fiber A - step 1")
    (yield)
    (println "Fiber A - step 2"))))

  (spawn (fiber (lambda []
    (println "Fiber B - step 1")
    (yield)
    (println "Fiber B - step 2"))))

  (run-fibers))
;; Output interleaved:
;; Fiber A - step 1
;; Fiber B - step 1
;; Fiber A - step 2
;; Fiber B - step 2
```

### Channels

Communicate between fibers:

```lisp
;; Create channel
(define ch (chan))

;; Or buffered channel
(define buffered-ch (chan 10))

;; Send and receive
(with-fibers
  ;; Producer
  (spawn (fiber (lambda []
    (send ch "message 1")
    (send ch "message 2")
    (send ch 'done))))

  ;; Consumer
  (spawn (fiber (lambda []
    (let loop []
      (let [msg (recv ch)]
        (when (!= msg 'done)
          (println (str "Got: " msg))
          (loop)))))))

  (run-fibers))
```

### Parallel Computation (HVM4)

Use HVM4's native parallelism:

```lisp
;; fork2 - parallel evaluation
(fork2 (expensive-computation-a)
       (expensive-computation-b))
;; Both run in parallel, returns when both complete

;; Superposition-based parallelism
(define parallel-map [f xs]
  (match xs
    [()]       '()
    [(h .. t)] (fork2 (cons (f h) nothing)
                      (parallel-map f t))))
```

## FFI (Foreign Function Interface)

Call native C functions from OmniLisp.

### Basic FFI Calls

```lisp
;; Load a library
(ffi-load "libc")

;; Call a function
(ffi-call libc "puts" "Hello from C!")

;; With return value
(define result (ffi-call libc "strlen" "hello"))
;; -> 5
```

### Handle-Based Safety

OmniLisp wraps native pointers in safe handles:

```lisp
;; Allocate memory
(define ptr (ffi-call libc "malloc" 1024))
;; ptr is a Handle, not a raw pointer

;; Use the handle
(ffi-call libc "memset" ptr 0 1024)

;; Free memory
(ffi-call libc "free" ptr)
;; Handle is invalidated after free
```

### Ownership Annotations

```lisp
;; Mark handle as owned (will be freed)
(define ^:owned ptr (ffi-call libc "malloc" 100))

;; Mark handle as borrowed (caller manages lifetime)
(define ^:borrowed view (get-buffer-view obj))

;; Consumed - handle invalidated after use
(define ^:consumed transfer (ffi-call lib "take_ownership" ptr))
```

### Defining FFI Bindings

```lisp
;; Define a typed FFI function
(define-ffi puts
  [lib "libc"]
  [name "puts"]
  [args [{String}]]
  [return {Int}])

;; Use like normal function
(puts "Hello!")

;; Struct definition for FFI
(define-ffi-struct Point
  [x {Int}]
  [y {Int}])
```

### Callbacks

```lisp
;; Create callback for C to call
(define my-callback
  (ffi-callback [{Int} {Int}] {Int}
    (lambda [a b] (+ a b))))

;; Pass to C function expecting function pointer
(ffi-call lib "register_callback" my-callback)
```

### Error Handling

```lisp
;; FFI calls can fail
(handle
  (let [result (ffi-call lib "dangerous_function" arg)]
    (process result))

  (ffi-error [err _]
    (println (str "FFI error: " err))
    nothing)

  (null-pointer [_ _]
    (println "Got null pointer")
    nothing))
```

## Proof-as-Effect System

OmniLisp includes experimental support for proofs as effects.

### Declaring Proofs

```lisp
;; Require a proof at runtime
(define safe-div [x {Int}] [y {Int}] {Int}
  (require-proof (not-zero? y))
  (/ x y))

;; Provide proof at call site
(with-proof (not-zero? 5)
  (safe-div 10 5))
```

### Proof Effects

```lisp
;; Proofs are effects that can be handled
(handle
  (safe-div x y)

  (proof-required [prop resume]
    (if (runtime-check prop)
        (resume 'proven)
        (perform fail "Proof obligation not satisfied"))))
```

## Best Practices

### Tower of Interpreters

1. **Use for code generation** - Not for simple abstractions
2. **Prefer macros when possible** - Less runtime overhead
3. **Test generated code** - Easy to introduce bugs
4. **Document stage boundaries** - Make it clear what runs when

### Macros

1. **Keep macros simple** - Complex macros are hard to debug
2. **Use gensym for hygiene** - Avoid variable capture
3. **Document expansion** - Show what macro expands to
4. **Test with macroexpand** - Verify expansion before use

### Fibers

1. **Don't share mutable state** - Use channels instead
2. **Always yield in loops** - Prevent starvation
3. **Handle fiber failures** - Fibers can crash
4. **Prefer channels over globals** - For communication

### FFI

1. **Always free allocated memory** - Prevent leaks
2. **Use handles, not raw pointers** - Safety layer
3. **Check return values** - C functions can fail
4. **Document ownership** - Who frees what?

## Further Reading

- [API Reference](API_REFERENCE.md) - Function reference
- [Quick Reference](QUICK_REFERENCE.md) - Language overview
- [Effects and Handlers](EFFECTS_AND_HANDLERS.md) - Effect system
- [MACRO_ARCHITECTURE.md](MACRO_ARCHITECTURE.md) - Macro implementation details

## References

- "Collapsing Towers of Interpreters" (Amin & Rompf, POPL 2018)
- "Hygienic Macro Expansion" (Kohlbecker et al., 1986)
- "Fibers: Cooperative Scheduling for MLton" (Fluet & Ford, 2008)
