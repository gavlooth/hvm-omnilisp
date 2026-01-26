# OmniLisp HVM4 Implementation TODO

**Last Updated:** 2026-01-25

---

## Overview

This document tracks the implementation progress of OmniLisp on HVM4. The goal is to port OmniLisp's semantics to HVM4 while leveraging HVM4's native parallelism and interaction net reduction.

### Completion Summary

```
Parser (Pika):        ████████████████████ 95%
Core Evaluator:       ████████████████░░░░ 80%
Type System:          ████░░░░░░░░░░░░░░░░ 20%
Standard Library:     ████████░░░░░░░░░░░░ 40%
FFI:                  ████████████░░░░░░░░ 60%
─────────────────────────────────────────────
Overall:              ████████████░░░░░░░░ 55%
```

---

## Completed Features

### Parser (Pika)
- [x] Core Pika parser implementation (`clang/omnilisp/pika/`)
- [x] Nick-encoded constructor names (`clang/omnilisp/nick/omnilisp.c`)
- [x] Basic expressions: lists, symbols, integers, strings
- [x] Type annotations `{Type}`
- [x] Slot syntax `[param {Type}]`
- [x] Metadata `^:key` and `^:key value`
- [x] Quote/quasiquote syntax `'x`, `` `x ``, `,x`, `,@x`
- [x] Spread patterns `.. rest`
- [x] Guard patterns `& condition`
- [x] Path expressions `obj.field`
- [x] Set literals `#{1 2 3}`
- [x] Dict literals `#{"key" value}`
- [x] Named characters `#\newline`, `#\space`
- [x] Format strings `#f"Hello {name}"`
- [x] Hash-val reader `#val`
- [x] Enhanced number parsing (signed, floats)

### Core Evaluator (HVM4 Runtime)
- [x] Lambda/closures (including recursive)
- [x] Let bindings (basic)
- [x] Conditionals (if)
- [x] Arithmetic operators (+, -, *, /, %)
- [x] Comparison operators (=, <, >, <=, >=)
- [x] Boolean operators (and, or, not)
- [x] Pattern matching (basic constructor patterns)
- [x] Delimited continuations (reset/control/shift)
- [x] Algebraic effects (handle/perform with CPS)
- [x] Fibers (CPS-based cooperative)
- [x] Fork/choice (HVM4 SUP-based)

### FFI
- [x] Handle table design
- [x] Basic thread pool
- [x] FFI node generation (`#FFI{name, args}`)

---

## Pending Features

### P0: Core Language (High Priority)

#### 1. Type System
**Status:** 20% - Basic annotations parsed, no runtime checking
**Reference:** `docs/TYPE_SYSTEM_DESIGN.md`, `docs/QUICK_REFERENCE.md` §3

```lisp
;; Abstract types
(define {abstract Number})
(define ^:parent {Number} {abstract Integer})

;; Struct types
(define {struct Point}
  [x {Int}]
  [y {Int}])

;; Parametric types
(define {struct [Pair T]}
  [first {T}]
  [second {T}])

;; Enum/sum types
(define {enum [Option T]}
  (Some [value {T}])
  None)

;; Union types
(define {IntOrString} (union [{Int} {String}]))
```

**Tasks:**
- [ ] Type registry in runtime
- [ ] `type?` predicate implementation
- [ ] Subtype checking (`^:parent` hierarchy)
- [ ] Parametric type instantiation
- [ ] Type inference for untyped bindings
- [ ] Runtime type validation on application

---

#### 2. Multiple Dispatch
**Status:** 0% - Not implemented
**Reference:** `docs/QUICK_REFERENCE.md` §2.6

```lisp
;; Same name, different arities
(define area 0)
(define area [r] (* 3.14 (* r r)))

;; Same name, different types
(define describe [x {Int}] "An integer")
(define describe [x {String}] "A string")

;; Combined
(define add [x {Int}] [y {Int}] (+ x y))
(define add [x {String}] [y {String}] (string-concat x y))
```

**Tasks:**
- [ ] Method table per function name
- [ ] Dispatch resolution (arity first, then type specificity)
- [ ] Most-specific-method selection algorithm
- [ ] Ambiguity detection and error reporting

---

#### 3. Macro System
**Status:** 0% - Design only
**Reference:** `docs/UNDOCUMENTED_FEATURES.md` §2

```lisp
;; Hygienic macro definition
(define [syntax when]
  [(when test body ...)
   (if test (do body ...) nil)])

;; With literal keywords
(define [syntax case]
  [literals else =>]
  [(case val (else result)) result]
  [(case val (datum => proc) clause ...)
   (if (eqv? val 'datum) (proc val) (case val clause ...))])
```

**Tasks:**
- [ ] `define [syntax ...]` parser support
- [ ] Pattern matching in macro expander
- [ ] `...` ellipsis handling (zero-or-more)
- [ ] Hygiene: gensym for introduced identifiers
- [ ] Macro expansion phase before evaluation

---

#### 4. Modules
**Status:** 0% - Not implemented
**Reference:** `docs/UNDOCUMENTED_FEATURES.md` §1

```lisp
(module Math
  (export sin cos tan sqrt)

  (define pi 3.14159)
  (define (sin x) ...)
  (define (cos x) ...))

(import Math)
(import Math :as M)
(import Math :only (sin cos))
```

**Tasks:**
- [ ] Module definition form
- [ ] Export list handling
- [ ] Import resolution
- [ ] Namespace isolation
- [ ] Aliased imports (`:as`)
- [ ] Selective imports (`:only`)

---

### P1: Standard Library (Medium Priority)

#### 5. Collections Runtime
**Status:** 60% - Basic ops exist, missing advanced
**Reference:** `docs/LANGUAGE_COMPLETENESS_REPORT.md` §3.2

**Implemented:**
- [x] map, filter, reduce, for-each, length, append
- [x] set, set-add, set-remove, set-contains?
- [x] dict, dict-get, dict-set, dict-keys, dict-values

**Missing:**
- [ ] `sort`, `sort-by`
- [ ] `group-by`
- [ ] `partition`
- [ ] `zip`, `zip-with`
- [ ] `flatten`
- [ ] `take`, `drop`, `take-while`, `drop-while`
- [ ] `find`, `find-index`
- [ ] `every?`, `any?`

---

#### 6. String Operations
**Status:** 80% - Most implemented
**Reference:** `docs/LANGUAGE_COMPLETENESS_REPORT.md` §3.1

**Implemented:**
- [x] string-upcase, string-downcase, string-capitalize
- [x] string-trim, string-pad-left/right
- [x] string-contains, string-index-of
- [x] string-replace, string-split, string-join
- [x] string-length, string-concat, string-substr

**Missing:**
- [ ] Ensure all ops work with HVM4 string representation
- [ ] Unicode support verification

---

#### 7. Regex Reader
**Status:** 0% - Parser syntax not implemented
**Reference:** `docs/QUICK_REFERENCE.md` §8

```lisp
;; Reader syntax
#r"[a-z]+"           ;; Compiles to Pika pattern

;; Equivalent to
(re-compile "[a-z]+")

;; Functions (delegate to Pika)
(re-match #r"\d+" "abc123")      ;; → "123"
(re-find-all #r"\w+" "a b c")    ;; → ("a" "b" "c")
(re-split #r"\s+" "a  b  c")     ;; → ("a" "b" "c")
(re-replace #r"\d" "X" "a1b2")   ;; → "aXbX"
(re-fullmatch #r"^\d+$" "123")   ;; → true
```

**Tasks:**
- [ ] Add `R_REGEX` rule to Pika parser
- [ ] `#r"..."` reader macro
- [ ] Compile regex to Pika grammar at read time
- [ ] `re-match`, `re-find-all`, `re-split`, `re-replace`, `re-fullmatch` runtime functions

---

#### 8. Pika Grammar DSL
**Status:** 0% - Parser support not implemented
**Reference:** `docs/UNDOCUMENTED_FEATURES.md` §3, `docs/QUICK_REFERENCE.md` §7

```lisp
(define [grammar arithmetic]
  [expr   (seq term (star (seq (or "+" "-") term)))]
  [term   (seq factor (star (seq (or "*" "/") factor)))]
  [factor (or number (seq "(" expr ")"))]
  [number (plus (range "0" "9"))])

;; Use the grammar
(pika-parse arithmetic "expr" "1+2*3")
```

**Tasks:**
- [ ] `define [grammar ...]` parser support
- [ ] Grammar AST to Pika rules compilation
- [ ] `pika-parse` runtime function
- [ ] Grammar composition (referencing other grammars)

---

#### 9. Quasiquote Evaluation
**Status:** 50% - Parsed, not fully evaluated
**Reference:** `docs/QUICK_REFERENCE.md` §Special Forms

```lisp
`(1 2 ,(+ 1 2))           ;; → (1 2 3)
`(1 ,@(list 2 3) 4)       ;; → (1 2 3 4)
`(a `(b ,(+ 1 2) ,,(+ 3 4)))  ;; Nested quasiquote
```

**Tasks:**
- [ ] `#QQ`, `#UQ`, `#UQS` evaluation in runtime
- [ ] Proper nesting level tracking
- [ ] Splice into list contexts

---

#### 10. Destructuring
**Status:** 30% - Basic patterns work
**Reference:** `docs/QUICK_REFERENCE.md` §2.4.1

```lisp
;; Sequence destructuring
(let [[x y z] my-list] ...)
(let [[x y .. rest] my-list] ...)

;; Dict/plist destructuring
(let [(:x :y) my-dict] ...)

;; In function params
(define process [[x y]] (+ x y))

;; In match
(match val
  [[a b .. rest] (cons a rest)]
  [_ nil])
```

**Tasks:**
- [ ] `.. rest` spread in let bindings
- [ ] Dict destructuring `(:key1 :key2)`
- [ ] Destructuring in function parameters
- [ ] Nested destructuring

---

#### 11. Loop/Recur (Iterative Construct)
**Status:** 0% - Not implemented
**Reference:** `docs/LANGUAGE_COMPLETENESS_REPORT.md` §Control Flow

```lisp
;; Basic loop with recur
(loop [i 0 acc 0]
  (if (>= i 10)
      acc
      (recur (+ i 1) (+ acc i))))
;; → 45 (sum 0..9)

;; Loop with early exit
(loop [xs my-list]
  (match xs
    [nil nil]
    [[h .. t] (if (= h target)
                  h
                  (recur t))]))
```

**Tasks:**
- [ ] `loop` special form parser support
- [ ] `recur` tail-call optimization
- [ ] Binding vector parsing `[var init ...]`
- [ ] Detect non-tail recur calls (error)

---

#### 12. Pipe Operator & Function Utilities
**Status:** 0% - Not implemented
**Reference:** `docs/LANGUAGE_COMPLETENESS_REPORT.md` §Functions

```lisp
;; Pipe/threading operator
(|> x
    (+ 1)
    (* 2)
    (- 10))
;; → (- (* (+ x 1) 2) 10)

;; Apply - call function with list of args
(apply + '(1 2 3 4))       ;; → 10
(apply f arg1 arg2 rest)   ;; → (f arg1 arg2 ...rest)

;; Partial application
(define add5 (partial + 5))
(add5 3)                   ;; → 8

;; Function composition
(define inc-then-double (comp (partial * 2) (partial + 1)))
(inc-then-double 3)        ;; → 8  ((3+1)*2)

;; Identity function
(identity x)               ;; → x

;; Constantly - returns function that always returns val
(define always-42 (constantly 42))
(always-42 "ignored")      ;; → 42
```

**Tasks:**
- [ ] `|>` macro/special form (thread-first)
- [ ] `apply` function
- [ ] `partial` higher-order function
- [ ] `comp` function composition
- [ ] `identity` function
- [ ] `constantly` function

---

#### 13. Conditional Forms (via Macros)
**Status:** 0% - Depends on macro system
**Reference:** `docs/LANGUAGE_COMPLETENESS_REPORT.md` §Control Flow

```lisp
;; when - execute body if test is truthy
(when (> x 0)
  (print "positive")
  x)

;; unless - execute body if test is falsy
(unless (nil? x)
  (process x))

;; cond - multi-way conditional
(cond
  [(< x 0) "negative"]
  [(= x 0) "zero"]
  [:else   "positive"])

;; case - value matching (different from match)
(case color
  [red   "#FF0000"]
  [green "#00FF00"]
  [blue  "#0000FF"]
  [_     "#000000"])
```

**Note:** These are typically implemented as macros once the macro system is ready.

**Tasks:**
- [ ] `when` macro
- [ ] `unless` macro
- [ ] `cond` macro
- [ ] `case` macro (symbol-based switching)

---

#### 14. Path Access & Mutation
**Status:** 30% - Parser done, runtime missing
**Reference:** `docs/QUICK_REFERENCE.md` §5, `docs/UNDOCUMENTED_FEATURES.md` §5

```lisp
;; Path Navigation (. operator)
person.name              ;; → (get person :name)
person.address.city      ;; → nested: (get (get person :address) :city)
arr.[0]                  ;; → (get arr 0)
matrix.[0].[1]           ;; → (get (get matrix 0) 1)

;; Mutation primitives
(put! person.age 31)           ;; Mutate nested field in-place
(update! counter.value inc)    ;; Apply fn to field in-place
(update person.age inc)        ;; Functional update (returns new object)

;; Manual path construction
(path obj :field1 :field2)     ;; Build path object programmatically
(get-in obj [:a :b :c])        ;; Get nested value
(assoc-in obj [:a :b] val)     ;; Set nested value (functional)
```

**Implemented:**
- [x] Parser: `R_PATH` rule for `obj.field` syntax

**Tasks:**
- [ ] `get` function for path navigation
- [ ] `put!` for in-place mutation
- [ ] `update!` for in-place fn application
- [ ] `update` for functional transformation
- [ ] `path` constructor
- [ ] `get-in` / `assoc-in` for nested access
- [ ] Chained path compilation (`a.b.c` → nested gets)

---

#### 15. Iterators (Lazy Sequences)
**Status:** 0% - Not implemented
**Reference:** `docs/QUICK_REFERENCE.md` §4

```lisp
(range 10)                    ;; Lazy 0..9
(map (fn [x] (* x 2)) iter)   ;; Lazy transform
(filter even? iter)           ;; Lazy filter
(take 5 iter)                 ;; Lazy take

(collect-list iter)           ;; Realize to list
(collect-array iter)          ;; Realize to array
```

**Tasks:**
- [ ] `T_ITER` type / `#Iter` constructor
- [ ] `range` implementation
- [ ] Lazy `map`, `filter`, `take`, `drop`
- [ ] `collect-list`, `collect-array`
- [ ] Iterator protocol (next, done?)

---

#### 16. Math Library
**Status:** 40% - Basic arithmetic only
**Reference:** `docs/LANGUAGE_COMPLETENESS_REPORT.md` §3.4

```lisp
;; Trigonometry
(sin x) (cos x) (tan x)
(asin x) (acos x) (atan x) (atan2 y x)

;; Powers/roots
(sqrt x) (pow x n) (exp x) (log x) (log10 x)

;; Rounding
(floor x) (ceil x) (round x) (truncate x)

;; Other
(abs x) (min a b) (max a b)
(random) (random-int n)
```

**Tasks:**
- [ ] FFI wrappers for libm functions
- [ ] `random` with seed support
- [ ] Ensure float representation works

---

#### 17. I/O System
**Status:** 30% - Basic file ops only
**Reference:** `docs/LANGUAGE_COMPLETENESS_REPORT.md` §3.5

```lisp
;; File operations
(read-file "path.txt")        ;; → string
(write-file "path.txt" data)  ;; → nil
(read-lines "path.txt")       ;; → list of strings
(append-file "path.txt" data)

;; Path operations
(path-join "a" "b" "c")       ;; → "a/b/c"
(path-dirname "/a/b/c")       ;; → "/a/b"
(path-basename "/a/b/c")      ;; → "c"
(path-exists? "path")
(path-is-file? "path")
(path-is-dir? "path")

;; Environment
(env-get "HOME")
(env-set "KEY" "value")
(env-keys)
```

**Tasks:**
- [ ] File read/write via FFI
- [ ] Path manipulation functions
- [ ] Environment variable access
- [ ] Directory listing

---

#### 18. JSON
**Status:** 0% - Not implemented
**Reference:** `docs/LANGUAGE_COMPLETENESS_REPORT.md` §3.6

```lisp
(json-parse "{\"a\": 1, \"b\": [2, 3]}")
;; → #{"a" 1 "b" [2 3]}

(json-stringify #{"name" "Alice" "age" 30})
;; → "{\"name\":\"Alice\",\"age\":30}"
```

**Tasks:**
- [ ] JSON parser (can use Pika grammar)
- [ ] JSON stringifier
- [ ] Handle nested structures
- [ ] Proper escaping

---

#### 19. DateTime
**Status:** 0% - Not implemented for HVM4
**Reference:** `docs/LANGUAGE_COMPLETENESS_REPORT.md` §3.3

```lisp
(datetime-now)
(datetime-now-utc)
(datetime-make 2026 1 25 14 30 0)
(datetime-year dt)
(datetime-add-days dt 7)
(datetime-diff dt1 dt2)
(datetime-format dt "%Y-%m-%d")
(datetime-parse-iso8601 "2026-01-25T14:30:00Z")
```

**Tasks:**
- [ ] DateTime representation in HVM4
- [ ] FFI to system time functions
- [ ] Format/parse functions
- [ ] Arithmetic operations

---

#### 20. Tower/Meta (Multi-Stage Programming)
**Status:** 0% - Not implemented for HVM4
**Reference:** `docs/UNDOCUMENTED_FEATURES.md` §4

```lisp
(lift val)           ;; value → code
(run code)           ;; execute code
(EM expr)            ;; meta-level evaluation
(clambda [x] body)   ;; compiled lambda
(meta-level)         ;; current tower level
```

**Tasks:**
- [ ] `#Cod` code representation
- [ ] `lift` implementation
- [ ] `run` compilation and execution
- [ ] `EM` meta-level jump
- [ ] Tower level tracking

---

### P2: Extended Features (Lower Priority)

#### 21. Networking
**Status:** 0% - Not implemented
**Reference:** `docs/LANGUAGE_COMPLETENESS_REPORT.md` §3.7

```lisp
;; TCP
(tcp-connect "host" port)
(tcp-listen port)
(tcp-accept server)
(tcp-send conn data)
(tcp-recv conn)
(tcp-close conn)

;; HTTP (high-level)
(http-get "https://example.com")
(http-post url body headers)
```

**Tasks:**
- [ ] TCP socket FFI
- [ ] UDP socket FFI
- [ ] HTTP client (can be built on TCP)
- [ ] Async I/O integration with fibers

---

#### 22. Developer Tools
**Status:** 20%
**Reference:** `docs/LANGUAGE_COMPLETENESS_REPORT.md` §4

**Missing:**
- [ ] `inspect` - object inspection
- [ ] `type-of` - runtime type query
- [ ] `doc` - documentation lookup
- [ ] `trace` - execution tracing
- [ ] `time` - timing macro
- [ ] `expand` - macro expansion display
- [ ] Testing framework (deftest, assert-eq)
- [ ] Profiling tools

---

## Architecture Notes

### Key Files

| Component | Location |
|-----------|----------|
| Pika Parser | `clang/omnilisp/pika/omni_pika.c` |
| Pika Header | `clang/omnilisp/pika/pika.h` |
| Nick Names | `clang/omnilisp/nick/omnilisp.c` |
| HVM4 Runtime | `lib/runtime.hvm4` (planned) |
| FFI | `clang/omnilisp/ffi/` (planned) |

### Design Principles

1. **All definitions use `define`** - No defmacro, defmethod, defgeneric
2. **Pika is NOT PEG** - Bottom-up table filling, handles left recursion
3. **Character Calculus**: `()` = execution, `[]` = slots, `{}` = types, `^` = metadata
4. **HVM4 native parallelism** - Use SUP for fork/choice, not threads
5. **Caller-controlled scheduling** - Fibers return state, no global scheduler

### HVM4 Constructors

```
#Lit{n}          ;; Literal integer
#Sym{nick}       ;; Symbol (nick-encoded)
#Var{idx}        ;; de Bruijn variable
#Lam{body}       ;; Lambda
#App{fn, arg}    ;; Application
#Let{val, body}  ;; Let binding
#If{c, t, e}     ;; Conditional
#Clo{env, body}  ;; Closure
#Kont{k}         ;; Continuation
#Fibr{s, k, m}   ;; Fiber
#FFI{name, args} ;; FFI call
#Hndl{idx, gen}  ;; Handle (FFI pointer)
```

---

## Original OmniLisp Tasks (Legacy)

These items were from the original C implementation and may need review for HVM4:

### REVIEWED:NAIVE Items

| Task | Location | Issue |
|------|----------|-------|
| Fix `prim_distinct` buffer overflow | `runtime/src/collections.c:932` | Fixed 1024-element buffer silently truncates |
| Replace bubble sort in profiler | `runtime/src/profile.c:393` | O(n²) bubble sort → qsort() |
| Hash-based condition lookup | `runtime/src/condition.c:68,78` | Linear search → hash table |
| Optimize generic method dispatch | `runtime/src/generic.c:214,296` | Linear search → dispatch table |

---

## References

- `docs/QUICK_REFERENCE.md` - Authoritative language reference
- `docs/SYNTAX.md` - Detailed syntax specification
- `docs/UNDOCUMENTED_FEATURES.md` - Additional features
- `docs/IMPLEMENTATION_COMPARISON.md` - Original vs HVM4 comparison
- `docs/LANGUAGE_COMPLETENESS_REPORT.md` - Original completion status
- `docs/PATTERN_SYNTAX.md` - Pika pattern matching details

---

## Changelog

### 2026-01-25
- Rewrote TODO.md for HVM4 implementation tracking
- Added comprehensive feature breakdown with code examples
- Completed Pika grammar rules for all syntax features
- Updated CLAUDE.md with Pika (not PEG) clarification

### 2026-01-23
- Original OmniLisp development complete
- See `archived_todos.md` for original development history
