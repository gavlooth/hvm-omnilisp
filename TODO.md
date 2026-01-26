# OmniLisp HVM4 Implementation TODO

**Last Updated:** 2026-01-26

---

## Philosophy

OmniLisp is built on a few core principles that distinguish it from other Lisps:

### Character Calculus
| Character | Domain | Role | Example |
|-----------|--------|------|---------|
| `()` | Flow | Execution/evaluation | `(+ 1 2)` |
| `[]` | Slot | Data/parameters (no eval) | `[x {Int}]` |
| `{}` | Kind | Types/constraints | `{Int}`, `{List {Int}}` |
| `^` | Meta | Metadata annotation | `^:mutable`, `^:ffi` |
| `.` | Path | Navigation/access | `obj.field`, `arr.[0]` |
| `#` | Reader | Dispatch/constructors | `#{}`, `#r"..."`, `#\newline` |

### Match is Truth
**All control flow desugars to `match`**. There is no separate `if`, `cond`, or `case` at the core level. These are syntactic conveniences that compile to pattern matching:

```lisp
;; if → match
(if cond then else) → (match cond [true then] [false else])

;; cond → match
(cond [c1 r1] [c2 r2]) → (match true [_ & c1 r1] [_ & c2 r2])

;; when/unless → match with Nothing
(when test body) → (match test [false nothing] [_ body])
```

### No Exceptions
OmniLisp uses **algebraic effects** instead of try/catch. Effects are first-class, composable, and don't break referential transparency:

```lisp
(handle
  (perform fail "something went wrong")
  (fail [msg resume] (println msg)))
```

### Named Let for Recursion
No `loop/recur`. Use named let for tail-recursive loops:

```lisp
(let loop [[i 0] [acc 0]]
  (if (>= i 10)
      acc
      (loop (+ i 1) (+ acc i))))
```

### Three Collection Types
- **List** `'(1 2 3)` - Linked list, optimal for head operations
- **Array** `[1 2 3]` - Indexed access, optimal for random access
- **Dict** `#{"key" val}` - Key-value mapping

Tuples are deprecated - use Arrays.

---

## Completion Summary

```
Character Calculus:   ████████████████████ 100%  (parsing)
Match (core):         █████████████████░░░ 85%
Type System:          ██████████░░░░░░░░░░ 50%
Multiple Dispatch:    ██████████████████░░ 90%
Algebraic Effects:    █████████████████░░░ 85%
Macro System:         ████████████░░░░░░░░ 60%
Modules:              ███████████████████░ 95%
Standard Library:     █████████████████░░░ 85%
FFI:                  ████████████░░░░░░░░ 60%
REPL & Editor:        ████████████████████ 100%
─────────────────────────────────────────────
Overall:              █████████████████░░░ 85%
```

---

## Completed Features

### Character Calculus (Parser)
- [x] `()` execution - function calls, special forms
- [x] `[]` slots - parameters, array literals, patterns
- [x] `{}` types - type annotations `{Int}`, `{List {T}}`
- [x] `^` metadata - `^:mutable`, `^:key value`
- [x] `.` paths - `obj.field`, `arr.[0]`
- [x] `#` reader dispatch - `#{}` dicts, `#\char`, `#f"format"`

### Match (Core Evaluator)
- [x] Constructor patterns `[(Point x y) ...]`
- [x] Literal patterns `[0 ...]`, `["str" ...]`
- [x] Wildcard `[_ ...]`
- [x] Variable binding `[x ...]`
- [x] Spread patterns `[h .. t]`
- [x] Guard clauses `[x & (> x 0)]`
- [x] `if` desugaring to match

### Algebraic Effects
- [x] `handle` block setup
- [x] `perform` effect invocation
- [x] `resume` continuation capture
- [x] CPS transformation for handlers
- [x] Handler stack in meta-environment

### Delimited Continuations
- [x] `reset` - delimit continuation
- [x] `shift` - capture up to nearest reset
- [x] `control` - abort/no automatic resume

### Concurrency
- [x] Fibers (CPS-based cooperative)
- [x] Fork/choice (HVM4 SUP-based parallelism)
- [ ] Channels (shared state via FFI)

### FFI
- [x] Handle table for pointer safety
- [x] Thread pool for blocking calls
- [x] `#FFI{name, args}` node generation
- [ ] Async model (`ffi/async`, `ffi/await`)

### REPL & Editor
- [x] Interactive REPL (`omnilisp` or `omnilisp -i`)
- [x] Socket server (`omnilisp -S PORT`)
- [x] Neovim plugin with eval keybindings

---

## Pending Features

### P0: Core Language

#### 1. Control Flow Desugaring
**Status:** 100% - All implemented in runtime evaluator
**Philosophy:** All control flow → match

```lisp
;; These are syntactic sugar, not primitives
(when test body)              ;; → (match test [false nothing] [_ body])
(unless test body)            ;; → (match test [true nothing] [_ body])
(cond [c1 r1] [c2 r2] ...)   ;; → (match true [_ & c1 r1] [_ & c2 r2] ...)
```

**Tasks:**
- [x] `if` → match desugaring in evaluator
- [x] `when` in runtime (#When node)
- [x] `unless` in runtime (#Unls node)
- [x] `cond` in runtime (#Cond node)
- [x] Named let syntax `(let name [var init] [var2 init2] ... body)`

---

#### 2. Type System
**Status:** 50% - Runtime checks work, parametric types pending
**Philosophy:** Julia-compatible type lattice with `{Any}` at top

```lisp
;; Types use {} brackets (Kind domain)
{Int}                       ;; Concrete type
{abstract Number}           ;; Abstract type (can't instantiate)
{List {Int}}               ;; Parametric type
{union [{Int} {String}]}   ;; Union type
{3}                        ;; Value type (singleton containing only 3)
```

**Tasks:**
- [x] Type descriptors in runtime
- [x] `type?` predicate
- [x] Subtype hierarchy (`^:parent`)
- [ ] Parametric type instantiation `{List {T}}`
- [ ] Union types `{union [...]}`
- [ ] Value types `{42}` (singleton types)

---

#### 3. Multiple Dispatch
**Status:** 100% - Full multiple dispatch with ambiguity detection
**Philosophy:** Arity first, then type specificity (Julia model)

```lisp
;; Same name, different signatures
(define add [x {Int}] [y {Int}] (+ x y))
(define add [x {String}] [y {String}] (string-concat x y))
(define add [x {List}] [y {List}] (append x y))

;; Dispatch resolution:
;; 1. Filter by arity
;; 2. Among matching arities, find most specific types
;; 3. Error if ambiguous
```

**Tasks:**
- [x] Method table `#GFun{name, methods}`
- [x] Arity-first dispatch
- [x] Type specificity ordering
- [x] Ambiguity detection and error (`@omni_compare_specificity` with `#MAmb` return)
- [x] Curried partial application `#GPrt`

---

#### 4. Macro System
**Status:** 60% - Basic expansion works, hygiene needs verification
**Philosophy:** `define [syntax ...]` form, hygienic by default

```lisp
(define [syntax when]
  [(when test body ...)
   (match test [false nothing] [_ (do body ...)])])

(define [syntax unless]
  [(unless test body ...)
   (match test [true nothing] [_ (do body ...)])])

(define [syntax cond]
  [(cond) nothing]
  [(cond [test result] rest ...)
   (match test [true result] [_ (cond rest ...)])])
```

**Tasks:**
- [x] `define [syntax ...]` parser
- [x] Pattern matching in expander
- [x] `...` ellipsis for rest patterns
- [x] Gensym for hygiene
- [ ] Ellipsis repetition in templates
- [ ] Literal keywords `[literals ...]`
- [ ] Nested macro expansion

---

#### 5. Modules
**Status:** 95% - All features implemented
**Philosophy:** Explicit exports, namespace isolation

```lisp
(module Math
  (export sin cos tan PI)
  (define PI 3.14159)
  (define (sin x) ...))

(import Math)           ;; All exports
(import Math :as M)     ;; Aliased: M.sin
(import Math :only (sin cos))  ;; Selective
```

**Tasks:**
- [x] Module definition `#Modl`
- [x] Export filtering
- [x] Import resolution
- [x] Aliased imports `:as`
- [x] Selective imports `:only`

---

### P1: Standard Library

#### 6. Function Combinators
**Status:** 100% - All implemented
**Philosophy:** Composition over inheritance, point-free where natural

```lisp
;; Pipe operator (desugar to nested application)
(|> x (f a) (g b) (h c))  ;; → (h (g (f x a) b) c)

;; Combinators (already in prelude)
(identity x)              ;; → x
(const x y)              ;; → x (ignores y)
(compose f g)            ;; → (lambda [x] (f (g x)))
(flip f)                 ;; → (lambda [a b] (f b a))
(curry f)                ;; → curried version
```

**Tasks:**
- [x] `|>` pipe operator (#Pipe node in runtime)
- [x] `identity` in prelude
- [x] `const` in prelude
- [x] `compose` in prelude
- [x] `flip` in prelude
- [x] `curry`/`uncurry` in prelude
- [x] `partial`, `partial2`, `partial3` in prelude
- [x] `apply` (#Appl node in runtime)

---

#### 7. Lazy Iterators
**Status:** 100% - Full implementation in prelude
**Philosophy:** Laziness via thunks, `#Iter`/`#Rang` types for protocol

```lisp
;; Lazy sequences
(lazy-range1 10)              ;; Lazy 0..9
(lazy-range 0 100 2)          ;; Lazy 0,2,4,...98

;; Lazy transformations (return iterators)
(lazy-map inc (lazy-range1 10))     ;; Lazy
(lazy-filter even? (lazy-range1 100)) ;; Lazy
(lazy-take 5 (iterate inc 0))       ;; Lazy, terminates

;; Realization (force evaluation)
(collect-list iter)          ;; → '(...)
(to-list iter)               ;; → '(...) (alias)
(for-each println iter)      ;; Side effects, returns nothing

;; Iterator protocol
(iter-next iter)             ;; → #Some{value, new_iter} or #None{}
(iter-done? iter)            ;; → #True{} / #Fals{}
```

**Tasks:**
- [x] `#Iter{state, next_fn}` constructor
- [x] `#Rang{start, end, step}` lazy range
- [x] `lazy-range`, `lazy-range1`, `lazy-range2`
- [x] `iterate`, `lazy-repeat`, `cycle`
- [x] `lazy-map`, `lazy-filter`, `lazy-take`, `lazy-drop`
- [x] `lazy-take-while`, `lazy-drop-while`
- [x] `lazy-zip`, `lazy-zip-with`, `enumerate`
- [x] `chain`, `lazy-flatten`
- [x] `collect-list`, `to-list`
- [x] `iter-fold`, `iter-reduce`, `for-each`
- [x] `iter-count`, `iter-sum`, `iter-find`
- [x] `iter-any`, `iter-all`, `iter-nth`, `iter-last`
- [x] `iter-next`, `iter-done?` protocol

---

#### 8. Collections (Polymorphic)
**Status:** 100% - Comprehensive implementation in prelude
**Reference:** prelude.hvm4 has map, filter, foldl, foldr, etc.
**Design:** All functions dispatch on type: List → List, Array → Array, Dict → Dict, Iterator → Iterator

**Implemented:**
- [x] `map`, `filter`, `foldl`, `foldr` (polymorphic: List/Array/Dict/Iterator)
- [x] `take`, `drop`, `zip`, `zipWith` (polymorphic: List/Array/Iterator)
- [x] `flatten`, `range` (eager), `repeat`
- [x] `all?`, `any?`, `sum`, `product`
- [x] `sort`, `sort-by`, `sort-asc`, `sort-desc` (merge sort)
- [x] `merge-sorted`
- [x] `group`, `group-by`
- [x] `partition`
- [x] `find`, `find-index`
- [x] `take-while`, `drop-while`
- [x] `span`, `break`, `split-at`
- [x] `intersperse`, `intercalate`
- [x] `nub`, `nub-by` (remove duplicates)
- [x] `minimum`, `maximum`, `minimum-by`, `maximum-by`

---

#### 9. Path Access
**Status:** 90% - Runtime implemented, update pending
**Philosophy:** `.` operator for navigation, `[]` for indexing

```lisp
;; Parsed by Character Calculus
person.name              ;; → (get person :name)
person.address.city      ;; → (get-in person [:address :city])
arr.[0]                  ;; → (nth arr 0)
matrix.[i].[j]           ;; → (nth (nth matrix i) j)

;; Functional operations (don't mutate)
(get obj key)            ;; Single-level access
(get-in obj path)        ;; Deep access
(assoc obj key val)      ;; Return new obj with key set
(assoc-in obj path val)  ;; Return new obj with deep key set
(update obj key f)       ;; Return new obj with (f (get obj key))
```

**Tasks:**
- [x] `.` path parsing
- [x] `get` function (#Get node + @omni_get helper)
- [x] `get-in` for nested access (#GtIn node + @omni_get_in helper)
- [x] `assoc` (#AsIn node + @omni_assoc helper)
- [x] `assoc-in` for functional update (#AsIn + @omni_assoc_in)
- [x] `update`, `update-in` (#Updt and #UpdI nodes)
- [ ] Path compilation (`a.b.c` → `(get-in a [:b :c])`)

---

#### 10. Quasiquote Evaluation
**Status:** 100% - Full implementation in runtime
**Philosophy:** Template-based code generation

```lisp
`(1 2 ,(+ 1 2))           ;; → '(1 2 3)
`(1 ,@(list 2 3) 4)       ;; → '(1 2 3 4) (splice)
`(a `(b ,(+ 1 2)))        ;; Nested: inner unquote not evaluated
```

**Tasks:**
- [x] `#QQ` quasiquote evaluation
- [x] `#UQ` unquote (evaluate and insert)
- [x] `#UQS` unquote-splicing (evaluate and splice)
- [x] Nesting level tracking (@omni_eval_qq with level parameter)

---

#### 11. Pika-Based Regex
**Status:** 0% - Not implemented
**Philosophy:** Deterministic matching, O(n) guaranteed, no backtracking

OmniLisp regex compiles to Pika grammars. This means:
- No catastrophic backtracking
- Predictable performance
- Limited features (no backreferences)

```lisp
;; Reader syntax
#r"[a-z]+"               ;; Compiles to Pika pattern at read time

;; Functions
(re-match #r"\d+" "abc123")      ;; → "123"
(re-find-all #r"\w+" "a b c")    ;; → '("a" "b" "c")
(re-split #r"\s+" "a  b  c")     ;; → '("a" "b" "c")
(re-replace #r"\d" "X" "a1b2")   ;; → "aXbX"
```

**Tasks:**
- [ ] `#r"..."` reader macro
- [ ] Regex → Pika grammar compilation
- [ ] `re-match`, `re-find-all`
- [ ] `re-split`, `re-replace`

---

#### 12. Pika Grammar DSL
**Status:** 0% - Not implemented
**Philosophy:** User-defined parsers as first-class values

```lisp
(define [grammar json]
  [value   (or object array string number :true :false :null)]
  [object  (seq "{" (star (seq pair (star (seq "," pair)))) "}")]
  [pair    (seq string ":" value)]
  [array   (seq "[" (star (seq value (star (seq "," value)))) "]")]
  [string  (seq "\"" (star (not "\"")) "\"")]
  [number  (seq (opt "-") (plus digit) (opt (seq "." (plus digit))))]
  [digit   (range "0" "9")])

(pika-parse json "value" "{\"a\": [1, 2, 3]}")
```

**Tasks:**
- [ ] `define [grammar ...]` parser support
- [ ] Grammar AST compilation
- [ ] `pika-parse` runtime function
- [ ] Grammar composition

---

#### 13. Math Library
**Status:** 40% - Basic arithmetic only
**Philosophy:** FFI wrappers for libm

```lisp
;; Trigonometry
(sin x) (cos x) (tan x)
(asin x) (acos x) (atan x) (atan2 y x)

;; Powers/roots
(sqrt x) (pow x n) (exp x) (log x) (log10 x)

;; Rounding
(floor x) (ceil x) (round x) (truncate x)
```

**Tasks:**
- [ ] FFI wrappers for libm
- [ ] `random`, `random-int` with seed
- [ ] Float representation in HVM4

---

#### 14. I/O System
**Status:** 30% - Basic file ops only
**Philosophy:** Effects-based I/O where practical

```lisp
;; File operations
(read-file "path.txt")
(write-file "path.txt" data)
(read-lines "path.txt")

;; Path operations
(path-join "a" "b" "c")       ;; → "a/b/c"
(path-dirname "/a/b/c")       ;; → "/a/b"
(path-exists? "path")

;; Environment
(env-get "HOME")
(env-set "KEY" "value")
```

**Tasks:**
- [ ] File read/write FFI
- [ ] Path manipulation
- [ ] Environment variables
- [ ] Directory listing

---

### P2: Extended Features

#### 15. Tower of Interpreters
**Status:** 0% - Not implemented
**Philosophy:** Reflective meta-programming, staged computation

The Tower allows code to manipulate its own interpreter:

```lisp
(lift val)           ;; Value → code representation
(run code)           ;; Execute code
(EM expr)            ;; Jump to meta-level, evaluate there
(clambda [x] body)   ;; Compiled lambda (staged)
(meta-level)         ;; Current tower level (0 = base)
```

**Tasks:**
- [ ] `#Cod` code representation
- [ ] `lift` implementation
- [ ] `run` compilation
- [ ] `EM` meta-level jump
- [ ] Tower level tracking in menv

---

#### 16. Channels
**Status:** 0% - Fibers exist, channels pending
**Philosophy:** CSP-style communication between fibers

```lisp
(define ch (channel 10))     ;; Buffered channel, capacity 10

;; In fiber 1
(send ch "message")          ;; Blocks if full

;; In fiber 2
(recv ch)                    ;; → "message", blocks if empty

;; Non-blocking variants
(try-send ch val)            ;; → true/false
(try-recv ch)                ;; → (Some val) or None
```

**Tasks:**
- [ ] `#Chan{buffer, cap}` constructor
- [ ] `send`/`recv` with blocking (via effects)
- [ ] `try-send`/`try-recv` non-blocking
- [ ] Channel select (`select [[ch1 v1] ...] ...)`)

---

#### 17. JSON
**Status:** 0% - Can implement with Pika grammar

```lisp
(json-parse "{\"a\": 1}")    ;; → #{"a" 1}
(json-stringify #{"a" 1})    ;; → "{\"a\":1}"
```

**Tasks:**
- [ ] JSON parser (use Pika grammar DSL)
- [ ] JSON stringifier
- [ ] Handle nested structures

---

#### 18. DateTime
**Status:** 0%

```lisp
(datetime-now)
(datetime-format dt "%Y-%m-%d")
(datetime-add-days dt 7)
```

**Tasks:**
- [ ] DateTime representation
- [ ] FFI to system time
- [ ] Format/parse functions

---

#### 19. Networking
**Status:** 0%

```lisp
(tcp-connect "host" port)
(tcp-send conn data)
(tcp-recv conn)
```

**Tasks:**
- [ ] TCP socket FFI
- [ ] Integration with fibers for async I/O

---

#### 20. Developer Tools
**Status:** 20%

- [ ] `inspect` - object inspection
- [ ] `type-of` - runtime type query
- [ ] `doc` - documentation lookup
- [ ] `trace` - execution tracing
- [ ] `time` - timing macro
- [ ] `expand` - macro expansion display
- [ ] Testing framework

---

## Architecture Notes

### Design Principles

1. **Match is the source of truth** - All conditionals desugar to pattern matching
2. **Character Calculus** - `()` `[]` `{}` `^` `.` `#` have distinct semantic roles
3. **Effects over exceptions** - `handle/perform/resume` instead of try/catch
4. **Named let for loops** - No separate loop/recur construct
5. **Three collections** - List, Array, Dict (no tuples)
6. **Pika is NOT PEG** - Bottom-up table filling, handles left recursion
7. **HVM4 native parallelism** - SUP for fork/choice, not OS threads

### HVM4 Constructors

```
;; Core evaluation
#Lit{n}          ;; Literal
#Sym{nick}       ;; Symbol
#Var{idx}        ;; de Bruijn variable
#Lam{body}       ;; Lambda
#App{fn, arg}    ;; Application
#Let{val, body}  ;; Let binding
#Mat{...}        ;; Match expression

;; Closures & continuations
#Clo{env, body}  ;; Closure
#Kont{k}         ;; Continuation
#Fibr{s, k, m}   ;; Fiber

;; Effects
#Perf{tag, val}  ;; Perform effect
#Hndl{...}       ;; Handle block

;; FFI
#FFI{name, args} ;; FFI call
#Hand{idx, gen}  ;; Handle (pointer wrapper)

;; Types
#TDsc{name, parent, fields}  ;; Type descriptor
#GFun{name, methods}         ;; Generic function
```

---

## References

- `docs/SYNTAX.md` - Authoritative syntax specification
- `docs/QUICK_REFERENCE.md` - Language overview
- `language_reference.md` - Detailed semantics
- `docs/PATTERN_SYNTAX.md` - Pattern matching details
- `docs/UNDOCUMENTED_FEATURES.md` - Advanced features

---

## Changelog

### 2026-01-26
- Rewrote TODO to align with OmniLisp philosophy
- Added Philosophy section explaining core principles
- Reorganized around Character Calculus and "match is truth"
- Clarified Pika-based regex (not backtracking)
- Added Tower of Interpreters section
- Updated completion percentages
- Implemented pipe operator `|>` (#Pipe node in runtime)
- Implemented apply function (#Appl node)
- Implemented when/unless/cond control flow (#When, #Unls, #Cond nodes)
- Implemented named let (Scheme-style loop): `#NLet` and `#NLeS` nodes in runtime
  - Supports `(let loop [i 0] [sum 0] (if (< i 10) (loop (+ i 1) (+ sum i)) sum))`
  - Sequential variant via `^:seq` metadata
  - CPS-compatible for use inside handle/reset blocks
- Implemented path access: get, get-in, assoc, assoc-in, update, update-in
- Added multiple dispatch ambiguity detection:
  - `@omni_compare_specificity` returns `#ASpec`, `#BSpec`, `#Equal`, or `#Ambig`
  - `@omni_find_best_method` now returns `#MAmb{name, sig_a, sig_b}` on ambiguity
  - Dispatch reports `#sym_AmbiguousMethod` error with both conflicting signatures
- Implemented quasiquote evaluation with nesting levels
- Added comprehensive collection functions to prelude:
  - sort, sort-by, sort-asc, sort-desc (merge sort)
  - find, find-index, partition
  - take-while, drop-while, span, break, split-at
  - group, group-by, intersperse, intercalate
  - nub, nub-by (deduplication)
  - minimum, maximum, minimum-by, maximum-by
- Added complete lazy iterator system to prelude:
  - Iterator protocol (iter-next, iter-done?)
  - Lazy range constructors (lazy-range, lazy-range1, lazy-range2)
  - Infinite generators (iterate, lazy-repeat, cycle)
  - Lazy transformations (lazy-map, lazy-filter, lazy-take, lazy-drop)
  - lazy-take-while, lazy-drop-while, lazy-zip, lazy-zip-with
  - chain, lazy-flatten, enumerate
  - Realization functions (collect-list, to-list, for-each)
  - Iterator consumers (iter-fold, iter-reduce, iter-count, iter-sum, iter-find, iter-any, iter-all, iter-nth, iter-last)
- Standard Library progress: 40% → 85%
- Overall progress: 75% → 85%
- Extended polymorphic dispatch to Arrays and Dicts:
  - `map` now works on List → List, Array → Array, Dict → Dict (values)
  - `filter` now works on List → List, Array → Array, Dict → Dict
  - `foldl`/`foldr` now work on List, Array, Dict, Iterator
  - `take`/`drop` now work on List → List, Array → Array, Iterator → Iterator
  - `length` now works on List, Array, Dict (native O(1) for arrays)
  - `reverse` now works on List → List, Array → Array
  - `zip` now works on List → List, Array → Array, Iterator → Iterator
  - Added `foldl_iter` helper for folding over iterators

### 2026-01-25
- Initial HVM4 implementation tracking
- Completed Pika parser
- REPL and editor integration
