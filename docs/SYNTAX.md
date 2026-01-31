# OmniLisp Syntax Reference

This document describes the complete syntax of OmniLisp, from lexical tokens to high-level expressions.

> **AUTHORITATIVE DIRECTIVE:** This document describes **OmniLisp syntax only**.
> Do NOT assume features from other Lisp dialects (Scheme, Common Lisp, Clojure, Racket, etc.).
> If a feature is not documented here, it does not exist in OmniLisp.
> When in doubt, check the Implementation Status section.

> **Implementation Status Note (2026-01-17):** This document describes the *design* syntax.
> See the [Implementation Status](#implementation-status) section at the end for what is
> currently implemented in the Pika parser (`csrc/parser/parser.c`).

## Table of Contents

1. [Character Calculus (Core Principle)](#character-calculus-core-principle)
2. [Lexical Syntax](#lexical-syntax)
3. [Primitive Types](#primitive-types)
4. [Collection Types](#collection-types)
5. [Expressions](#expressions)
6. [Definitions](#definitions)
7. [Pattern Matching (Primary Control Flow)](#pattern-matching-primary-control-flow)
8. [Type Annotations](#type-annotations)
9. [Algebraic Effects](#algebraic-effects)
10. [Delimited Continuations](#delimited-continuations)
11. [Special Forms](#special-forms)
12. [Macros & Staging](#macros--staging)
13. [Metadata](#metadata)
14. [Reader Macros](#reader-macros)
15. [Standard Library](#standard-library)
16. [Implementation Status](#implementation-status)

---

## Character Calculus (Core Principle)

OmniLisp adheres to a strict **Character Calculus** that gives each bracket type a specific domain:

| Character | Domain | Purpose |
| :--- | :--- | :--- |
| **`{}`** | **Kind** | Type annotations, blueprints, and the domain of static blueprints. |
| **`[]`** | **Slot** | Arguments, parameters, field lists, and data-carrying sequences (Data Domain). |
| **`()`** | **Flow** | Execution, value construction, and type-generating calculations (Execution Domain). |
| **`^`**  | **Metadata** | Out-of-band instructions, relationships, and constraints. |

This principle guides all syntax decisions:
- Types go in braces: `{Int}`, `{String}`, `{List T}`
- Data goes in brackets: `[1 2 3]`, `[x {Int}]`
- Execution goes in parens: `(+ 1 2)`, `(define ...)`
- Metadata uses caret: `^:mutable`, `^:where`

---

## Lexical Syntax

### Whitespace

- **Spaces** (`U+0020`): Significant for separating tokens
- **Tabs** (`U+0009`): Treated as spaces
- **Newlines** (`U+000A`, `U+000D`): Treated as whitespace
- **Comments**: Start with `;` and continue to end of line

### Numbers

#### Integers
```
<integer> ::= [+-]? [0-9]+
```

Examples: `42`, `0`, `-123`, `+456`

#### Floats
```
<float> ::= [+-]? ( [0-9]+ "." [0-9]* | [0-9]* "." [0-9]+ )
```

Examples: `3.14`, `-0.5`, `.5`, `3.`, `+2.718`

### Symbol Character Rules

Symbols in OmniLisp follow specific character rules to maintain clarity and avoid ambiguity with other syntax.

**Start with (first character):**
- Letters: `a-z`, `A-Z`
- Operators: `*`, `!`, `-`, `_`, `?`, `%`, `/`, `=`, `<`, `>`

**Excluded from start:**
- Digits: `0-9` (to avoid confusion with integers)
- Reserved syntax: `.`, `@`, `#`, `&`, `:`, `;`

**Middle/subsequent characters:**
- All of the above (letters + operators)
- **Plus:** Digits `0-9`

**Excluded entirely:**
- `.` - Used for module paths (`Math.sin`)
- `@` - Used for metadata (`^:where`)
- `#` - Used for reader macros (`#val`, `#\newline`)
- `&` - Excluded (rest patterns use `..` spread operator)
- `:` - Used for type annotations (`{Type}`) and colon-quoted symbols
- `;` - Used for comments

**Examples:**

```lisp
; Valid symbols
foo                ; letters
foo-bar            ; - as separator
foo123             ; digits in middle
x1_y2              ; _ as separator
set!               ; ! at end
null?              ; ? at end
!not               ; ! at start
?maybe             ; ? at start
*                  ; single operator
<=                 ; comparison operators
==                 ; equality

; Invalid (can't start with digits)
123foo             ; integer, not symbol
3d                 ; digit first

; Invalid (reserved for syntax)
.foo               ; . for paths
foo.bar            ; . for paths (not a single symbol)
@meta              ; @ for metadata
#reader            ; # for reader macros
:type              ; : for colon-quoted symbols
```

### Strings

Strings are delimited by double quotes (`"`) and support escape sequences.

```
<string> ::= '"' <string-char>* '"'
```

Escape sequences:
- `\"` - Double quote
- `\\` - Backslash
- `\n` - Newline
- `\t` - Tab
- `\r` - Carriage return
- `\xNN` - Hex character code

Examples: `"hello"`, `"world\n"`, `"escaped \"quote\""`, `"\x41"` (= "A")

### Characters

Character literals use the `#\` prefix.

```
<char> ::= '#\' <named-char>
        |  '#\' 'x' <hex> <hex>
```

Named characters:
- `#\newline` - Newline (U+000A)
- `#\space` - Space (U+0020)
- `#\tab` - Tab (U+0009)

Hex characters:
- `#\x41` - Character with hex code 41 ('A')
- `#\x00` - Null character

### Colon-Quoted Symbols (`:name`)

OmniLisp does **not** have a separate "keyword" type.
Instead, `:name` is **pure reader sugar** for a quoted symbol:

```
:name  ≡  'name  ≡  (quote name)
```

Grammar:
```
<colon-quoted-symbol> ::= ':' <symbol>
```

Examples: `:foo`, `:my-keyword`

---

## Primitive Types

| Type | Description | Examples |
|------|-------------|----------|
| **Integer** | Arbitrary precision integers | `42`, `-123`, `0` |
| **Float** | Double-precision floating point | `3.14`, `-0.5`, `1.0` |
| **String** | Sequence of characters | `"hello"` |
| **Character** | Single Unicode codepoint | `#\a`, `#\newline` |
| **Symbol** | Identifier/symbol | `'foo`, `bar` |
| **Boolean** | True or false | `true`, `false` |
| **Nothing** | Singleton "no value" value | `nothing` |

---

## Collection Types

OmniLisp uses exactly **3 core collection types**:

| Type | Syntax | Character | Use Case |
|------|--------|-----------|----------|
| **List** | `(1 2 3)` or `'(1 2 3)` | `()` Flow | Cons cells, code representation, recursive processing |
| **Array** | `[1 2 3]` | `[]` Slot | Mutable, indexed access, general sequences |
| **Dict** | `#{:a 1 :b 2}` | `#{}` | Key-value storage, structured data |

> **DESIGN DECISION (2026-01-15):** Tuples are deprecated. Use Arrays instead.
> Named tuples are deprecated. Use Dicts instead.
> This simplifies the mental model while providing equivalent functionality.

### Lists

Lists use `()` and represent cons cells (linked lists).

```lisp
()                    ; empty list
'()                   ; quoted empty list
'(1 2 3)              ; list of integers
(list 1 2 3)          ; same, using function
'(a b c)              ; list of symbols
```

Lists are the fundamental data structure for code representation (S-expressions).

### Arrays

Arrays use `[]` and provide mutable, indexed access.

```lisp
[]                   ; empty array
[1 2 3]              ; array of integers
["a" "b" "c"]        ; array of strings
[[1 2] [3 4]]        ; nested arrays
```

Arrays grow dynamically and support O(1) index access.

### Dictionaries

Dictionaries use `#{}` and provide key-value storage.

```lisp
#{}                  ; empty dictionary
#{:a 1 :b 2}         ; dictionary with symbol keys
#{:name "Alice" :age 30}
```

Keys are typically colon-quoted symbols (`:key`), which are equivalent to `'key`.

---

## Expressions

### Literals

```lisp
42          ; integer
-123        ; negative integer
3.14        ; float
"hello"     ; string
#\a         ; character
'foo        ; symbol
true        ; boolean true
false       ; boolean false
nothing     ; nothing value
```

### Function Application

```lisp
(+ 1 2)              ; addition
(print "hello")      ; function call
(map f xs)           ; higher-order function
```

### Path Expressions

Paths use `.` to access fields and module members.

```lisp
object.field
module.submodule.function
person.address.city
```

---

## Definitions

### Function Definitions

OmniLisp uses **Slot syntax** for function parameters: each parameter is in `[]` with optional type in `{}`.

#### Canonical Form (Recommended)

```lisp
;; With types
(define add [x {Int}] [y {Int}] {Int}
  (+ x y))

;; Without types
(define square [x]
  (* x x))

;; Mixed types
(define process [x] [y {Int}] [z {Float}]
  (body))
```

#### Shorthand Form (Desugars to Canonical)

```lisp
;; Shorthand - bare symbols become untyped slots
(define add x y
  (+ x y))

;; Equivalent to:
(define add [x] [y]
  (+ x y))
```

### Variable Definitions

```lisp
(define x 42)
(define name "Alice")
(define counter 0)
```

### Typed Variables

```lisp
(define x {Int} 42)
(define name {String} "Alice")
```

---

## Pattern Matching (Primary Control Flow)

**`match` is the single source of truth for all conditional logic in OmniLisp.**

All other conditional forms (`if`, `when`, `unless`) are syntactic sugar that desugar to `match` at parse time. This design:
- Simplifies the runtime (only one control flow mechanism)
- Enables powerful pattern-based dispatch
- Leverages HVM4's native pattern matching for efficiency

### Basic Patterns

```lisp
(match value
  1  "one"
  2  "two"
  _  "other")
```

### Variable Binding Patterns

```lisp
(match value
  x  (* x 2))         ; binds x to value
```

### Destructuring Patterns

```lisp
(match [1 2 3]
  [x y z]  (+ x y z))       ; binds x=1, y=2, z=3

(match [[1 2] 3]
  [[a b] c]  (+ a b c))     ; nested destructuring
```

### As Patterns

```lisp
(match [1 2]
  [x y] as pair  pair)      ; binds x=1, y=2, pair=[1 2]
```

### Guards

```lisp
(match value
  x & (> x 0)  "positive"
  x & (< x 0)  "negative"
  _            "zero")
```

### Rest Patterns

```lisp
(match [1 2 3 4 5]
  [x y .. rest]  rest)      ; binds x=1, y=2, rest=[3 4 5]
```

---

## Type Annotations

Types are annotated using `{}` (Kind domain).

### Basic Annotations

```lisp
(define x {Int} 42)
(define name {String} "Alice")
```

### Function Type Annotations

```lisp
(define add [x {Int}] [y {Int}] {Int}
  (+ x y))

(define process [x {String}] {Int}
  (string-length x))
```

### Type Parameters

> **NOTE:** OmniLisp does NOT have `deftype`, `defstruct`, `defenum`.
> All type definitions use `define` with slot placeholders.

```lisp
;; Type parameter in first slot
(define Pair [T] [first {T}] [second {T}])

;; ADT with type parameter
(define Maybe [T]
  Nothing
  (Just [val {T}]))
```

### Type Constraints (`^:where`)

```lisp
;; x and y must be the same type T, where T is a Number
(define add ^:where [T {Number}]
  [x {T}] [y {T}] {T}
  (+ x y))
```

### Value Types (Singleton Types)

Value types constrain a type to a single literal value. Syntax: `{literal}` = `#val literal`

```lisp
;; Value type syntax
{3}           ; Type containing only value 3
{true}        ; Type containing only true
{"hello"}     ; Type containing only "hello"
{#\a}         ; Type containing only character 'a'
{1.5}         ; Type containing only 1.5

;; Equivalence with #val
{42}          ; same as #val 42
{false}       ; same as #val false
```

**Use in patterns:**
```lisp
(match x
  n {0}    "zero"
  n {1}    "one"
  n {Int}  "other")
```

---

## Algebraic Effects

OmniLisp uses **algebraic effects** for error handling and control flow.

> **IMPORTANT:** OmniLisp does NOT support `try`/`catch`. Use algebraic effects instead.

### Effect Declaration

```lisp
;; Declare an effect type
(define {effect Error}
  [raise [msg {String}] {bottom}])

;; Resumable effect
(define {effect Ask}
  [ask [prompt {String}] {String}])
```

### Performing Effects

Effects are performed like normal function calls:

```lisp
(define safe-div [x {Int}] [y {Int}] {Int}
  (if (= y 0)
    (raise "Division by zero")   ; perform effect
    (/ x y)))
```

### Handling Effects

```lisp
(handle
  (safe-div 10 0)
  [raise [msg] (println "Error: " msg) 0])    ; handle and return default

;; With resumption
(handle
  (ask "Name?")
  [ask [prompt] (resume "Alice")])            ; resume with value
```

### Effect Operation Semantics

**Key Insight:** Effect operations are **NOT** special forms. They are ordinary function calls
that are dynamically intercepted by the nearest enclosing `handle` form.

```lisp
;; `raise` is NOT a keyword - it's a normal function call
(raise "Division by zero")

;; This is equivalent to:
(perform 'raise "Division by zero")
```

**How it works:**
1. When an effect operation is called (e.g., `(raise msg)`), the runtime searches
   the call stack for a matching handler
2. The `handle` form installs a **prompt** (delimiter) with handler clauses
3. When an effect matches a handler clause, the continuation is captured up to that prompt
4. The handler clause receives the payload and optionally a **resumption** object

### Effect Row Type Signatures

Functions can declare which effects they may perform using the `^:effects` metadata:

```lisp
;; Function that may raise Error effect
(define safe-div [x {Int}] [y {Int}] {Int} ^:effects [{Error}]
  (if (= y 0)
    (raise "Division by zero")
    (/ x y)))

;; Function with multiple effects
(define interactive-compute [x {Int}] {Int} ^:effects [{Ask} {Emit}]
  (let [name (ask "Your name?")]
    (emit (string-append "Hello, " name))
    (* x 2)))

;; Pure function (no effects)
(define double [x {Int}] {Int}
  (* x 2))
```

**Effect rows are checked at compile time:**
- Calling an effectful function from a pure context is a type error
- Handlers must cover all effects that may be performed
- Effect inference tracks which effects flow through a computation

### Resume Semantics

The `resume` function is a **normal function** that is only valid within handler clauses.
It invokes the captured continuation (resumption) with a value.

**Type signature:**
```
resume : (Resumption a → a → b)
```

**Semantics:**
1. `resume` is passed as the second argument to handler clauses (after payload)
2. Calling `resume` continues the computation from where the effect was performed
3. The value passed to `resume` becomes the return value of the effect operation
4. Handlers can call `resume` zero times (abort), once (one-shot), or multiple times (multi-shot)

```lisp
;; Handler that does NOT resume (aborts computation)
(handle
  (begin (raise "error") (println "never reached"))
  [raise [msg] 0])  ; returns 0, computation aborted

;; Handler that resumes once (one-shot continuation)
(handle
  (+ 1 (ask "number?"))
  [ask [prompt resume] (resume 41)])  ; returns 42

;; Handler that resumes multiple times (multi-shot)
(handle
  (emit (choose [1 2 3]))
  [choose [options resume]
    (map resume options)])  ; calls resume for each option
```

**Implementation note:** Resumptions are implemented using delimited continuations.
The `handle` form installs a prompt, and `resume` invokes `cont_invoke()` on the
captured continuation.

### Why Not try/catch?

Algebraic effects provide:
1. **Type Safety**: Effects are part of the type signature
2. **Resumption**: Handlers can resume computation (impossible with exceptions)
3. **Composability**: Multiple effects compose without nested try blocks
4. **Explicit Control Flow**: No hidden control flow jumps
5. **Region Compatibility**: Works naturally with Region-RC memory management

---

## Delimited Continuations

OmniLisp provides first-class delimited continuations for advanced control flow. These are the foundation for algebraic effects and fiber-based concurrency.

### `reset` - Establish Continuation Boundary

```lisp
(reset body)

;; Creates a delimiter (prompt) for continuation capture
(reset (+ 1 (control k (k (k 10)))))  ; → 12
```

### `control` - Capture Continuation

```lisp
(control k body)

;; Captures the continuation up to the nearest reset
;; k is bound to the continuation function

(reset
  (+ 1 (control k
         (k 10))))  ; k = (lambda [x] (+ 1 x))
                    ; → 11

;; Multi-shot: call continuation multiple times
(reset
  (list (control k (list (k 1) (k 2) (k 3)))))
  ; → ((1) (2) (3))

;; Abort: don't call continuation
(reset
  (+ 1 (control k 42)))  ; → 42 (continuation discarded)
```

### `shift` - Alternative Continuation Capture

```lisp
(shift k body)

;; Same as control, but with slightly different semantics
;; (Scheme-style shift/reset)

(reset
  (+ 1 (shift k (k (k 10)))))  ; → 12
```

### `yield` - Fiber/Coroutine Yield

```lisp
(yield value)

;; Suspends the current fiber, returning value to the scheduler
;; Used with fibers and structured concurrency

(define producer []
  (yield 1)
  (yield 2)
  (yield 3))
```

### Continuation Use Cases

```lisp
;; 1. Implement generators
(define (make-generator thunk)
  (let [cont nothing]
    (lambda []
      (reset
        (if (nothing? cont)
            (thunk)
            (cont nothing))))))

;; 2. Implement async/await pattern
(define (async-fetch url)
  (control k
    (http-get url (lambda [result] (k result)))))

;; 3. Implement backtracking
(define (amb choices)
  (control k
    (for-each k choices)))

;; 4. Implement coroutines
(define (coroutine thunk)
  (reset (thunk)))
```

---

## Special Forms

### Conditionals (Syntactic Sugar for Match)

All conditional forms desugar to `match` at parse time. They exist for convenience and readability.

#### `if` - Binary Conditional
```lisp
(if condition then-expr else-expr)

;; Desugars to:
(match condition
  true   then-expr
  _      else-expr)
```

#### `when` - Single-Branch Conditional
```lisp
(when condition body...)

;; Desugars to:
(match condition
  true   body...
  _      nothing)
```

#### `unless` - Negated Conditional
```lisp
(unless condition body...)

;; Desugars to:
(match condition
  true   nothing
  _      body...)
```

**Why syntactic sugar?** Using `match` as the single control flow primitive:
- Simplifies the HVM4 runtime (one mechanism to optimize)
- Enables pattern matching in all conditionals
- Makes the language more consistent and predictable

### Let Bindings

Let bindings use Slot `[]` syntax: `[name {Type}? value]`

```lisp
;; Untyped bindings
(let [x 10] [y 20]
  (+ x y))

;; Typed bindings
(let [x {Int} 10] [y {Int} 20]
  (+ x y))

;; Sequential let (each binding sees previous ones)
(let ^:seq [x 1] [y (+ x 1)]
  y)

;; Destructuring in let
(let [[a b] my-array]
  (+ a b))
```

### Named Let (Scheme-Style Iteration)

Named let provides a way to define a local recursive function, commonly used for iteration:

```lisp
;; Named let: (let name [bindings...] body)
;; The 'name' becomes a function callable within body

;; Sum integers from 0 to 9
(let loop [i 0] [sum 0]
  (if (< i 10)
      (loop (+ i 1) (+ sum i))  ; recursive call
      sum))                      ; → 45

;; Build a list in reverse
(let collect [i 0] [acc '()]
  (if (< i 5)
      (collect (+ i 1) (cons i acc))
      acc))                      ; → (4 3 2 1 0)

;; Factorial
(let fact [n 10] [acc 1]
  (if (<= n 1)
      acc
      (fact (- n 1) (* acc n)))) ; → 3628800
```

**Note:** OmniLisp does NOT have Clojure-style `loop`/`recur`. Use named let instead.

### Parallel vs Sequential Evaluation

By default, `let` bindings evaluate in **parallel** (HVM4 native parallelism). Use `^:seq` for sequential evaluation when bindings depend on each other:

```lisp
;; PARALLEL (default) - bindings evaluate simultaneously
;; Good when bindings are independent
(let [a (expensive-compute 1)]
     [b (expensive-compute 2)]
     [c (expensive-compute 3)]
  (+ a b c))  ; a, b, c computed in parallel

;; SEQUENTIAL (^:seq) - bindings evaluate left-to-right
;; Required when later bindings depend on earlier ones
(let ^:seq [x 1]
           [y (+ x 1)]    ; y depends on x
           [z (* y 2)]    ; z depends on y
  z)  ; → 4

;; Named let with ^:seq
(let ^:seq loop [i 0] [sum 0]
  (if (< i 10)
      (loop (+ i 1) (+ sum i))
      sum))
```

| Form | Evaluation | Use When |
|------|------------|----------|
| `(let ...)` | Parallel | Bindings are independent |
| `(let ^:seq ...)` | Sequential | Bindings depend on each other |

### Lambdas

```lisp
(lambda [x] (* x x))
(fn [x] (* x x))
(λ [x] (* x x))

;; Multiple parameters
(fn [x] [y] (+ x y))

;; Typed parameters
(fn [x {Int}] [y {Int}] {Int}
  (+ x y))
```

### Quote

```lisp
'foo                  ; quoted symbol
'(1 2 3)              ; quoted list
`(a ,b c)             ; quasiquote with unquote
```

### Pipe Operator (`|>`)

Thread a value through a series of single-argument functions.

```lisp
;; Basic pipe (prefix syntax)
(|> 5 inc)                  ; → (inc 5) → 6

;; Chained functions
(|> 5 inc square)           ; → (square (inc 5)) → 36

;; Desugars to nested application
(|> x f g h)                ; → (h (g (f x)))
```

### Function Combinators

Combinators for argument manipulation with pipes and higher-order functions.

```lisp
;; curry: convert multi-arg to curried form
((curry +) 5 10)            ; → 15
(|> 5 (curry +) 10)         ; → 15

;; curry with explicit arity (for multi-arity functions)
(curry f 2)                 ; curry the 2-arg version of f

;; flip: swap first two arguments
((flip -) 5 10)             ; → (- 10 5) → 5
(|> 5 (flip -) 10)          ; → 5

;; rotate: cycle args left (first to end)
((rotate f) a b c)          ; → (f b c a)

;; comp: compose functions right-to-left
((comp square inc) 5)       ; → 36
```

| Combinator | Effect | Use Case |
|------------|--------|----------|
| `(curry f)` | Multi-arg → curried | Partial application in pipes |
| `(curry f n)` | Curry specific arity | Multi-arity functions |
| `(flip f)` | Swap first two args | Piped value as 2nd arg |
| `(rotate f)` | First arg to end | Piped value as last arg |
| `(comp f g)` | Right-to-left compose | Function chains |

---

## Macros & Staging

OmniLisp supports hygienic macros and multi-stage programming for metaprogramming.

### Syntax Macros

Define macros with pattern → template rules:

```lisp
;; Basic macro definition
(define [syntax when]
  [(when ?cond ?body ...)
   (match ?cond
     true  (do ?body ...)
     _     nothing)])

;; Usage
(when (> x 0)
  (print "positive")
  (process x))
```

### Pattern Variables

Pattern variables capture parts of the input:

| Syntax | Meaning | Example |
|--------|---------|---------|
| `?name` | Capture single form | `?cond` matches `(> x 0)` |
| `?name ...` | Capture zero or more | `?body ...` matches `(a) (b) (c)` |
| `literal` | Match exactly | `else` matches only `else` |

### Ellipsis Patterns

The `...` ellipsis matches zero or more forms:

```lisp
;; Match multiple body forms
(define [syntax my-begin]
  [(my-begin ?body ...)
   (do ?body ...)])

(my-begin (print "a") (print "b") (print "c"))
;; Expands to: (do (print "a") (print "b") (print "c"))

;; Recursive patterns with ellipsis
(define [syntax my-and]
  [(my-and) true]
  [(my-and ?x) ?x]
  [(my-and ?x ?rest ...)
   (if ?x (my-and ?rest ...) false)])

(my-and a b c)
;; Expands to: (if a (if b c false) false)
```

### Literal Keywords

Specify keywords that must match exactly (not as pattern variables):

```lisp
;; 'else' is a literal keyword
(define [syntax my-cond]
  [literals else]
  [(my-cond (else ?result)) ?result]
  [(my-cond (?test ?result) ?rest ...)
   (if ?test ?result (my-cond ?rest ...))])

(my-cond
  ((= x 0) "zero")
  ((> x 0) "positive")
  (else "negative"))
```

### Hygiene

OmniLisp macros are **hygienic by default** using a mark-based system. Variables introduced by macros don't capture variables in user code:

```lisp
;; Macro introduces 'temp' - won't capture user's 'temp'
(define [syntax swap!]
  [(swap! ?a ?b)
   (let [temp ?a]
     (do (set! ?a ?b)
         (set! ?b temp)))])

;; User's 'temp' is safe
(let [temp 100]
  (let [x 1] [y 2]
    (swap! x y)
    temp))  ;; → 100 (not captured)
```

### Gensym for Explicit Hygiene

Generate unique symbols when needed:

```lisp
;; gensym creates unique symbols
(gensym "temp")  ;; → temp_g42 (unique each call)

;; Use in macros for guaranteed uniqueness
(define [syntax with-temp]
  [(with-temp ?body)
   (let [g (gensym "t")]
     `(let [,g 0] ,?body))])
```

### Quasiquote in Macros

Build code with `` ` `` (quasiquote), `,` (unquote), and `,@` (unquote-splicing):

```lisp
;; Quasiquote builds code structure
`(+ 1 ,(+ 2 3))        ;; → '(+ 1 5)
`(list ,@'(a b c))     ;; → '(list a b c)

;; In macro templates
(define [syntax make-adder]
  [(make-adder ?n)
   `(lambda [x] (+ x ,?n))])

((make-adder 5) 10)    ;; → 15
```

### Macro Expansion

```lisp
;; Expand macro once
(expand-1 '(when (> x 0) (print x)))
;; → (match (> x 0) true (do (print x)) _ nothing)

;; Fully expand all macros recursively
(expand '(when (> x 0) (print x)))
;; → (match (> x 0) true (do (print x)) _ nothing)

;; macroexpand-all for deep expansion
(macroexpand-all '(my-and a (my-or b c)))
```

### Common Macro Patterns

```lisp
;; Short-circuit and/or
(define [syntax and]
  [(and) true]
  [(and ?x) ?x]
  [(and ?x ?rest ...) (if ?x (and ?rest ...) false)])

(define [syntax or]
  [(or) false]
  [(or ?x) ?x]
  [(or ?x ?rest ...)
   (let [temp ?x] (if temp temp (or ?rest ...)))])

;; Thread-first (->)
(define [syntax ->]
  [(-> ?x) ?x]
  [(-> ?x (?f ?args ...) ?rest ...)
   (-> (?f ?x ?args ...) ?rest ...)]
  [(-> ?x ?f ?rest ...)
   (-> (?f ?x) ?rest ...)])

;; let* via macro
(define [syntax let*]
  [(let* () ?body) ?body]
  [(let* ([?var ?val] ?rest ...) ?body)
   (let [?var ?val] (let* (?rest ...) ?body))])
```

### Grammar Definitions (Pika-based)

```lisp
;; Define a mini-language/DSL using Pika grammar rules
(define [grammar json-value]
  json-null    := "null"                    → nothing
  json-bool    := "true" / "false"          → (= $0 "true")
  json-number  := [0-9]+                    → (string->int $0)
  json-string  := "\"" [^"]* "\""           → (substr $0 1 -1)
  json-array   := "[" (json-value ",")* "]" → (list $1 ...)
  json-object  := "{" (pair ",")* "}"       → (dict $1 ...))
```

### Staged Computation

```lisp
;; eval-meta: evaluate at meta level (compile time)
(eval-meta '(+ 1 2))  ; → 3 at compile time

;; staged-fn: create staged function
(define fast-pow (staged-fn [n]
  (lambda [x]
    (eval-meta
      (fold (lambda [acc _] `(* ,acc x)) 'x (range n))))))

;; (fast-pow 3) generates at compile time:
;; (lambda [x] (* (* (* x x) x)))
```

---

## Metadata

Metadata uses the `^` prefix and provides out-of-band instructions.

### Type Metadata

```lisp
^:parent {Number}        ; inheritance
^:where [T {Number}]     ; type constraints
^:mutable                ; mutability marker
^:covar                  ; covariance marker
```

### Evaluation Metadata

| Metadata | Applies To | Effect |
|----------|-----------|--------|
| `^:strict` | `let` bindings | Forces eager evaluation |
| `^:seq` | `let` bindings | Sequential (ordered) evaluation |
| `^:parallel` | `let` bindings | Parallel evaluation with dependency analysis |
| `^:pure` | `define` | Marks function as pure (no side effects) |
| `^:speculate` | `match`, `if` | Enables speculative branch execution |
| `^:effects` | `define` | Declares effect row for function |

#### Strict Evaluation (`^:strict`)

By default, `let` bindings are lazy. Use `^:strict` to force immediate evaluation:

```lisp
;; Lazy (default) - expr evaluated when x is used
(let [x (expensive-computation)]
  (if condition x 0))

;; Strict - expr evaluated immediately
(let ^:strict [x (expensive-computation)]
  (if condition x 0))

;; Combine with ^:seq for strict sequential
(let ^:strict ^:seq
  [x (compute-x)]       ; evaluated first
  [y (compute-y x)]     ; evaluated second, after x
  (combine x y))
```

**HVM4 Compilation:**
- Lazy: `!x = expr;` (thunk)
- Strict: `!!&x = expr;` (forced evaluation)

#### Parallel Let (`^:parallel`)

Enables automatic dependency analysis and parallel scheduling:

```lisp
(let ^:parallel
  [a (compute-a)]           ; independent
  [b (compute-b)]           ; independent
  [c (combine a b)]         ; depends on a, b
  c)
;; a and b run in parallel, c waits for both
```

#### Pure Functions (`^:pure`)

Marks function as having no side effects:

```lisp
(define ^:pure square [x] (* x x))

;; Enables:
;; - Parallel execution in map/filter
;; - Memoization
;; - Common subexpression elimination
```

#### Speculative Execution (`^:speculate`)

Evaluates branches speculatively in parallel:

```lisp
;; Both branches start executing; unused one discarded
(if ^:speculate condition
  (expensive-then)
  (expensive-else))

;; Speculative match - all arms evaluated in parallel
(match ^:speculate value
  pattern1 result1
  pattern2 result2)
```

#### Effect Rows (`^:effects`)

Declares what effects a function may perform:

```lisp
(define log-value [x]
  ^:effects [IO]
  (do (println x) x))

(define ^:pure add [x] [y] (+ x y))
;; Equivalent to: ^:effects []
```

### Definition Metadata

```lisp
;; Mutable type definition
(define ^:mutable Player
  [hp {Int32}]
  [name {String}])

;; FFI function
(define ^:ffi sqrt [x {Float64}] {Float64})
```

---

## Reader Macros

Special syntax starting with `#`:

```lisp
#val 42              ; value-to-type conversion (singleton type)
#{:a 1 :b 2}         ; dictionary literal
#\newline            ; character literal
#fmt"Hello $x"       ; format string (experimental)
```

---

## Comments

Comments start with `;` and continue to end of line.

```lisp
; This is a comment
(define x 42)  ; inline comment
```

---

## Standard Library

OmniLisp includes a comprehensive standard library. For complete details, see `docs/STDLIB.md`.

### Math Functions

```lisp
;; Basic
(+ a b), (- a b), (* a b), (/ a b), (mod a b), (abs x)

;; Rounding
(floor x), (ceil x), (round x), (truncate x)

;; Powers & Roots
(sqrt x), (pow base exp), (exp x), (log x), (log10 x)

;; Trigonometry
(sin x), (cos x), (tan x), (asin x), (acos x), (atan x), (atan2 y x)

;; Constants
pi, e, inf, -inf, nan
```

### String Functions

```lisp
;; Construction & Access
(string-length s), (string-ref s i), (string-concat s1 s2 ...)
(substring s start end), (string-join sep strings)

;; Case
(string-upcase s), (string-downcase s), (string-capitalize s)

;; Search & Replace
(string-contains s sub), (string-index-of s sub)
(string-replace s old new), (string-split s sep)

;; Trim & Pad
(string-trim s), (string-pad-left s n char), (string-pad-right s n char)

;; Regex (Pika-based)
(re-match pattern s), (re-find-all pattern s)
(re-replace pattern s replacement), (re-split pattern s)
```

### Collection Functions

Collection operations use **gradual multiple dispatch** - the same function works on Lists, Arrays, and Iterators:

```lisp
;; Generic Operations (dispatch on collection type)
(map f coll)           ; List → List, Array → Array, Iterator → lazy
(filter pred coll)     ; Same dispatch behavior
(reduce f init coll)   ; Aka fold
(take n coll)          ; First n elements
(drop n coll)          ; Skip first n elements
(find pred coll)       ; First matching element
(any? pred coll)       ; True if any element matches
(all? pred coll)       ; True if all elements match

;; Eager-only Operations
(for-each f coll), (length coll), (append coll1 coll2)
(nth coll i), (first coll), (rest coll), (last coll)
(reverse coll), (sort coll), (sort-by f coll)

;; Iterator-specific (lazy wrappers)
(enumerate iter)       ; Pairs with index
(zip iter1 iter2 ...)  ; Combine iterators
(chain iter1 iter2)    ; Concatenate iterators
(cycle iter)           ; Infinite repetition
(iter-next iter)       ; Pull next value
(iter-peek iter)       ; Look without consuming

;; Type constructors (also convert via dispatch)
(list iter)            ; Collect to list, or (list 1 2 3)
(array iter)           ; Collect to array, or (array 1 2 3)
(set iter)             ; Collect to set, or (set 1 2 3)
(dict pairs)           ; From pairs, or (dict :a 1 :b 2)
```

**Generic Collection Access (gradual dispatch):**
```lisp
(get coll key)         ; Works on Dict, Array, List, String
(get coll key default) ; With default value
(put coll key val)     ; Functional update (returns new coll)
(keys coll)            ; Dict keys, or indices for Array
(values coll)          ; Dict values, or elements for Array
(contains? coll val)   ; Membership test for any collection
(merge coll1 coll2)    ; Merge dicts, concat lists/arrays
```

**Set-Specific Operations:**
```lisp
(add s elem)           ; Add element (generic, also works on lists)
(remove s elem)        ; Remove element
(union s1 s2)          ; Set union
(intersection s1 s2)   ; Set intersection
(difference s1 s2)     ; Set difference
```

### File I/O

```lisp
;; Reading
(read-file path), (read-lines path), (file-exists? path)

;; Writing
(write-file path content), (append-file path content)

;; Directory Operations
(list-dir path), (mkdir path), (delete-file path)
(file-info path)  ; returns dict with size, mtime, etc.
```

### JSON

```lisp
(json-parse string)      ; String → OmniLisp value
(json-encode value)      ; OmniLisp value → String
(json-encode-pretty val) ; Pretty-printed output
```

### DateTime

```lisp
;; Current Time
(datetime-now), (datetime-now-utc)

;; Construction
(datetime-make year month day hour min sec)
(datetime-from-unix timestamp)

;; Accessors
(datetime-year dt), (datetime-month dt), (datetime-day dt)
(datetime-hour dt), (datetime-minute dt), (datetime-second dt)

;; Arithmetic
(datetime-add-days dt n), (datetime-add-hours dt n)
(datetime-diff dt1 dt2)  ; returns duration

;; Formatting
(datetime-format dt pattern), (datetime-to-iso8601 dt)
(datetime-parse-iso8601 string)
```

### Networking

```lisp
;; HTTP Client
(http-get url), (http-post url body), (http-request method url opts)

;; TCP Sockets
(socket-connect host port), (socket-listen port)
(socket-send sock data), (socket-recv sock n)
(socket-close sock)
```

### Environment & System

```lisp
(getenv name), (setenv name value)
(exit code)
(command-line-args)  ; returns list of args
```

### Reflection & Debugging

```lisp
(type-of value)     ; returns type as symbol
(inspect value)     ; detailed structure info
(doc symbol)        ; documentation string
(source symbol)     ; source code if available
```

---

## Implementation Status

This section documents which syntax features are implemented in the Pika parser
(`csrc/parser/parser.c`) vs the analyzer (`csrc/analysis/analysis.c`).

### Fully Implemented (Parser)

| Feature | Parser Rule | Notes |
|---------|-------------|-------|
| Integers | `R_INT` | Positive and signed (`-123`, `+456`) |
| Floats | `R_FLOAT` | Full format including `.5` and `3.` |
| Symbols | `R_SYM` | Includes `-`, `+`, `*`, etc. as valid start chars |
| Strings | `R_STRING` | With escape sequences `\n`, `\t`, `\"`, `\\`, `\xNN` |
| Lists | `R_LIST` | S-expressions `(...)` |
| Arrays | `R_ARRAY` | Bracket syntax `[...]` |
| Dicts | `R_DICT` | Hash-brace syntax `#{...}` |
| Type literals | `R_TYPE` | Brace syntax `{Type}` or value types `{3}`, `{true}` |
| Paths | `R_PATH` | Dot notation `foo.bar.baz` |
| Quote | `R_QUOTED` | `'expr`, `` `expr ``, `,expr`, `,@expr` |
| Colon-quoted | `R_COLON_QUOTED_SYMBOL` | `:name` → `(quote name)` |
| Metadata keys | `R_META_KEY` | `^:parent`, `^:where`, etc. |
| Metadata attach | `R_METADATA` | `^ meta obj` (requires whitespace) |
| Named chars | `R_NAMED_CHAR` | `#\newline`, `#\space`, `#\tab`, `#\xNN` |
| Format strings | `R_FMT_STRING` | `#fmt"..."` |
| Value-to-type | `R_HASH_VAL` | `#val 42` → `(value->type 42)` |
| Comments | `R_COMMENT` | `;` to end of line |
| Match clauses | Codegen | Flat pairs: `pattern result` and `pattern & guard result` |

### Analyzer-Level Recognition

These are parsed as plain symbols but recognized specially in the analyzer:

| Symbol | Recognition | Location |
|--------|-------------|----------|
| `true` | Boolean true | `analysis.c`, `codegen.c` |
| `false` | Boolean false | `analysis.c`, `codegen.c` |
| `nothing` | Nothing value | `parser.c` (parser special-case) |
| `nil` | Nil value | `parser.c` (parser special-case) |
| `lambda` | Lambda form | `parse/_.c` |
| `fn` | Lambda alias | `parse/_.c` |
| `λ` (U+03BB) | Lambda alias | `parse/_.c` (UTF-8: 0xCE 0xBB) |

### Partial Implementation

| Feature | Status | Notes |
|---------|--------|-------|
| Effect declarations | Done | `(define {effect Name} [op [args] {RetType}])` |
| Effect row signatures | Done | `^:effects [{Error} {Ask}]` metadata |
| Variance annotations | Done | `^:covar`, `^:contravar` for type params |
| `handle` form | Codegen done | Works via runtime primitives |
| Pattern bindings | Done | Variable extraction in match patterns |
| As patterns | Done | `pattern as name` syntax |
| Guards | Done | `&` in match clauses: `pattern & guard result` |
| Rest patterns | Done | `[x y .. rest]` syntax |
| Type constraints | Done | `^:where [T {Number}]` in define |
| Format strings | Done | `#fmt"Hello $name"` with `$var` and `${expr}` interpolation |
| Set literals | Done | `#set{1 2 3}` reader macro |
| Staged evaluation | Done | `eval-meta`, `staged-fn`, `clambda` |
| Grammar definitions | Done | `(define [grammar name] rule := pattern → action)` full DSL |

### NOT IMPLEMENTED

| Feature | Status |
|---------|--------|
| `try`/`catch` | **Intentionally NOT supported** - use algebraic effects |
| Tuples | **Deprecated** - use arrays |
| Named Tuples | **Deprecated** - use dicts |

---

## Quick Reference Summary

```lisp
;; Collections (ONLY these 3)
'(1 2 3)              ; List
[1 2 3]               ; Array
#{:a 1 :b 2}          ; Dict

;; Definitions
(define x 42)                          ; variable
(define add [x] [y] (+ x y))           ; function (slot syntax)
(define add [x {Int}] {Int} (* x x))   ; typed function

;; Pattern Matching (flat pairs, & for guards, or for alternatives)
(match value
  pattern          result
  pattern & guard  result
  [x y] as pair    (use pair)
  (or a b)         result)

;; Algebraic Effects (NOT try/catch)
(handle
  (might-fail x)
  [raise [msg] default-value])

;; Types in {}
{Int}  {String}  {List T}  {Array Int}

;; Metadata with ^
^:mutable  ^:where [T {Number}]  ^:parent {Real}
```

---

## PEG Grammar Comparison (Original vs HVM4)

This section compares the Pika parser grammar rules between the original OmniLisp
implementation and the HVM4-OmniLisp implementation.

### Grammar Rule Summary

| Category | Original OmniLisp | HVM4 OmniLisp | Status |
|----------|-------------------|---------------|--------|
| **Whitespace** | | | |
| Space/Tab/Newline | `R_CHAR_SPACE`, `R_CHAR_TAB`, `R_CHAR_NL` | + `R_CHAR_CR` | ✅ Extended |
| Whitespace sequence | `R_SPACE`, `R_WS` | `R_SPACE`, `R_WS` | ✅ Same |
| Comments | ❌ Missing | `R_SEMICOLON`, `R_COMMENT_CHAR`, `R_COMMENT_INNER`, `R_COMMENT` | ✅ Added |
| Skip (ws + comments) | ❌ Missing | `R_WS_OR_COMMENT`, `R_SKIP` | ✅ Added |
| **Numbers** | | | |
| Digits | `R_DIGIT`, `R_DIGIT1` | `R_DIGIT`, `R_DIGIT_NZ`, `R_DIGITS` | ✅ Extended |
| Integer | `R_INT` | `R_INT`, `R_SIGNED_INT` | ✅ Extended |
| Float | ❌ Missing | `R_DOT`, `R_FRAC`, `R_FLOAT`, `R_FLOAT_FULL`, `R_FLOAT_LEAD`, `R_FLOAT_TRAIL`, `R_ANY_FLOAT` | ✅ Added |
| Sign | ❌ Missing | `R_OPT_SIGN` (optional +/-) | ✅ Added |
| Number (int\|float) | ❌ Missing | `R_NUMBER` | ✅ Added |
| **Symbols** | | | |
| Alpha (a-z) | `R_ALPHA` (a-z only) | `R_ALPHA_LOWER`, `R_ALPHA_UPPER`, `R_ALPHA` | ✅ Fixed |
| Special chars | ❌ Incomplete | `R_SYM_PLUS`, `R_SYM_MINUS`, `R_SYM_STAR`, `R_SYM_SLASH`, `R_SYM_EQ`, `R_SYM_LT`, `R_SYM_GT`, `R_SYM_BANG`, `R_SYM_QMARK`, `R_SYM_UNDER`, `R_SYM_AT`, `R_SYM_PERCENT`, `R_SYM_AMP` | ✅ Added |
| Symbol char | `R_SYM_CHAR` (stub) | `R_SYM_SPECIAL`, `R_SYM_INIT`, `R_SYM_CHAR`, `R_SYM_CONT` | ✅ Complete |
| Symbol | `R_SYM` | `R_SYM` | ✅ Improved |
| Colon-quoted | ❌ Missing | `R_COLON`, `R_COLON_SYM` | ✅ Added |
| **Strings** | | | |
| Quote char | ❌ Missing | `R_DQUOTE` | ✅ Added |
| Escape sequences | ❌ Missing | `R_BACKSLASH`, `R_ESC_N`, `R_ESC_T`, `R_ESC_R`, `R_ESC_QUOTE`, `R_ESC_BSLASH`, `R_ESC_CHAR`, `R_ESCAPE_SEQ` | ✅ Added |
| String | ❌ Missing | `R_STRING_CHAR`, `R_STRING_INNER`, `R_STRING` | ✅ Added |
| **Character Literals** | | | |
| Named chars | ❌ Missing | `R_CHAR_NEWLINE`, `R_CHAR_TAB_NAME`, `R_CHAR_SPACE_NAME`, `R_CHAR_RETURN`, `R_CHAR_NAMED` | ✅ Added |
| Char literal | ❌ Missing | `R_CHAR_BACKSLASH`, `R_CHAR_SIMPLE`, `R_CHAR_LITERAL` | ✅ Added |
| **Delimiters** | | | |
| Parens | `R_LPAREN`, `R_RPAREN` | `R_LPAREN`, `R_RPAREN` | ✅ Same |
| Brackets | `R_LBRACKET`, `R_RBRACKET` | `R_LBRACKET`, `R_RBRACKET` | ✅ Same |
| Braces | `R_LBRACE`, `R_RBRACE` | `R_LBRACE`, `R_RBRACE` | ✅ Same |
| Hash-brace | `R_HASHBRACE` | `R_HASHBRACE` | ✅ Same |
| Caret | ❌ Missing | `R_CARET` | ✅ Added |
| Spread | ❌ Missing | `R_DOTDOT` | ✅ Added |
| Guard | ❌ Missing | `R_COLONWHEN` | ✅ Added |
| **Compound Forms** | | | |
| List | `R_LIST_INNER`, `R_LIST` | `R_LIST_INNER`, `R_LIST` | ✅ Same |
| Array/Slot | `R_ARRAY_INNER`, `R_ARRAY` | `R_SLOT_INNER`, `R_SLOT` | ✅ Renamed |
| Dict | `R_DICT_INNER`, `R_DICT` | `R_DICT_INNER`, `R_DICT` | ✅ Same |
| Type | `R_TYPE_INNER`, `R_TYPE` | `R_TYPE_INNER`, `R_TYPE` | ✅ Same |
| **Special Forms** | | | |
| Metadata | ❌ Missing | `R_META_KEY`, `R_META` | ✅ Added |
| Guard expr | ❌ Missing | `R_GUARD` | ✅ Added |
| Spread pattern | ❌ Missing | `R_SPREAD` | ✅ Added |
| **Quote/Quasiquote** | | | |
| Quote char | ❌ Missing | `R_QUOTE_CHAR` (') | ✅ Added |
| Quasiquote | ❌ Missing | `R_QUASIQUOTE_CHAR` (`) | ✅ Added |
| Unquote | ❌ Missing | `R_UNQUOTE_CHAR` (,), `R_UNQUOTE_SPLICE` (,@) | ✅ Added |
| Quoted expr | ❌ Missing | `R_QUOTED` | ✅ Added |
| **Path Expressions** | | | |
| Path segment | ❌ Missing | `R_PATH_SEGMENT` | ✅ Added |
| Path tail | ❌ Missing | `R_PATH_TAIL_ITEM`, `R_PATH_TAIL` | ✅ Added |
| Path | ❌ Missing | `R_PATH_ROOT`, `R_PATH` | ✅ Added |
| **Set Literal** | | | |
| Set | ❌ Missing | `R_HASHSET`, `R_SET` (#set{...}) | ✅ Added |
| **Named Characters** | | | |
| Hash prefix | ❌ Missing | `R_HASH` | ✅ Added |
| Named char | ❌ Missing | `R_NAMED_CHAR` (#\newline, #\xNN) | ✅ Added |
| **Format Strings** | | | |
| Fmt string | ❌ Missing | `R_HASH_FMT`, `R_FMT_STRING` (#fmt"...") | ✅ Added |
| CLF string | ❌ Missing | `R_HASH_CLF`, `R_CLF_STRING` (#clf"...") | ✅ Added |
| **Hash-val Reader** | | | |
| Hash-val | ❌ Missing | `R_HASH_VAL` (#val atom) | ✅ Added |
| **Kind Splice** | | | |
| Kind splice | ❌ Missing | `R_HASHKIND`, `R_KIND_SPLICE` ({#kind expr}) | ✅ Added |
| **Atoms** | | | |
| Atom | ❌ Missing | `R_ATOM` (number \| string \| symbol) | ✅ Added |
| **Root** | | | |
| Expression | `R_EXPR` | `R_EXPR` (expanded with 19 alternatives) | ✅ Extended |
| Program | `R_ROOT` | `R_PROGRAM_INNER`, `R_PROGRAM` | ✅ Improved |

### Pika Rule Types

Pika uses a specific rule type system (not standard PEG notation):

| Pika Type | Notation | Description |
|-----------|----------|-------------|
| `PIKA_TERMINAL` | `"str"` | Literal string match |
| `PIKA_RANGE` | `[a-z]` | Character range |
| `PIKA_ANY` | `.` | Any single character |
| `PIKA_SEQ` | `A B C` | Sequence (all must match) |
| `PIKA_ALT` | `A / B / C` | Prioritized choice (first match wins) |
| `PIKA_REP` | `A*` | Zero-or-more |
| `PIKA_POS` | `A+` | One-or-more |
| `PIKA_OPT` | `A?` | Optional (zero-or-one) |
| `PIKA_NOT` | `!A` | Negative lookahead (doesn't consume) |
| `PIKA_AND` | `&A` | Positive lookahead (doesn't consume) |
| `PIKA_REF` | `→ RuleID` | Reference to another rule by ID |

### Pika Grammar Definition (HVM4 OmniLisp)

```
; === Whitespace and Comments ===
R_CHAR_SPACE   = TERMINAL " "
R_CHAR_TAB     = TERMINAL "\t"
R_CHAR_NL      = TERMINAL "\n"
R_CHAR_CR      = TERMINAL "\r"
R_SPACE        = ALT(R_CHAR_SPACE, R_CHAR_TAB, R_CHAR_NL, R_CHAR_CR)
R_WS           = REP(R_SPACE)                              ; whitespace*

R_SEMICOLON    = TERMINAL ";"
R_COMMENT_CHAR = RANGE(' ', '~')                           ; printable ASCII
R_COMMENT_INNER= REP(R_COMMENT_CHAR)
R_COMMENT      = SEQ(R_SEMICOLON, R_COMMENT_INNER)         ; ; ...
R_WS_OR_COMMENT= ALT(R_SPACE, R_COMMENT)
R_SKIP         = REP(R_WS_OR_COMMENT)                      ; (ws | comment)*

; === Numbers ===
R_DIGIT        = RANGE('0', '9')
R_DIGIT_NZ     = RANGE('1', '9')
R_DIGITS       = POS(R_DIGIT)                              ; digit+
R_DOT          = TERMINAL "."
R_OPT_SIGN     = ALT(R_SYM_PLUS, R_SYM_MINUS)              ; + / -
R_INT          = POS(R_DIGIT)                              ; digit+
R_SIGNED_INT   = SEQ(R_OPT_SIGN, R_DIGITS)                 ; [+-] digit+
R_FRAC         = SEQ(R_DOT, R_DIGITS)                      ; . digit+
R_FLOAT        = SEQ(R_DIGITS, R_FRAC)                     ; digit+ . digit+
R_FLOAT_FULL   = SEQ(R_OPT_SIGN, R_DIGITS, R_DOT, R_DIGITS); [+-]? digit+ . digit+
R_FLOAT_LEAD   = SEQ(R_OPT_SIGN, R_DOT, R_DIGITS)          ; [+-]? . digit+ (.5)
R_FLOAT_TRAIL  = SEQ(R_OPT_SIGN, R_DIGITS, R_DOT)          ; [+-]? digit+ . (3.)
R_ANY_FLOAT    = ALT(R_FLOAT_FULL, R_FLOAT_LEAD, R_FLOAT_TRAIL)
R_NUMBER       = ALT(R_ANY_FLOAT, R_SIGNED_INT, R_INT)     ; float / signed / int

; === Symbols ===
R_ALPHA_LOWER  = RANGE('a', 'z')
R_ALPHA_UPPER  = RANGE('A', 'Z')
R_ALPHA        = ALT(R_ALPHA_LOWER, R_ALPHA_UPPER)

R_SYM_PLUS     = TERMINAL "+"
R_SYM_MINUS    = TERMINAL "-"
R_SYM_STAR     = TERMINAL "*"
R_SYM_SLASH    = TERMINAL "/"
R_SYM_EQ       = TERMINAL "="
R_SYM_LT       = TERMINAL "<"
R_SYM_GT       = TERMINAL ">"
R_SYM_BANG     = TERMINAL "!"
R_SYM_QMARK    = TERMINAL "?"
R_SYM_UNDER    = TERMINAL "_"
R_SYM_AT       = TERMINAL "@"
R_SYM_PERCENT  = TERMINAL "%"
R_SYM_AMP      = TERMINAL "&"

R_SYM_SPECIAL  = ALT(R_SYM_PLUS, R_SYM_MINUS, R_SYM_STAR, R_SYM_SLASH,
                     R_SYM_EQ, R_SYM_LT, R_SYM_GT, R_SYM_BANG,
                     R_SYM_QMARK, R_SYM_UNDER, R_SYM_AT, R_SYM_PERCENT, R_SYM_AMP)
R_SYM_INIT     = ALT(R_ALPHA, R_SYM_SPECIAL)               ; first char
R_SYM_CHAR     = ALT(R_ALPHA, R_DIGIT, R_SYM_SPECIAL)      ; continuation char
R_SYM_CONT     = REP(R_SYM_CHAR)                           ; rest of symbol
R_SYM          = SEQ(R_SYM_INIT, R_SYM_CONT)               ; symbol

R_COLON        = TERMINAL ":"
R_COLON_SYM    = SEQ(R_COLON, R_SYM)                       ; :name → (quote name)

; === Strings ===
R_DQUOTE       = TERMINAL "\""
R_BACKSLASH    = TERMINAL "\\"
R_ESC_N        = TERMINAL "n"
R_ESC_T        = TERMINAL "t"
R_ESC_R        = TERMINAL "r"
R_ESC_QUOTE    = TERMINAL "\""
R_ESC_BSLASH   = TERMINAL "\\"
R_ESC_CHAR     = ALT(R_ESC_N, R_ESC_T, R_ESC_R, R_ESC_QUOTE, R_ESC_BSLASH)
R_ESCAPE_SEQ   = SEQ(R_BACKSLASH, R_ESC_CHAR)              ; \n \t \r \" \\
R_STRING_CHAR  = ALT(R_ESCAPE_SEQ, R_ALPHA)                ; simplified
R_STRING_INNER = REP(R_STRING_CHAR)
R_STRING       = SEQ(R_DQUOTE, R_STRING_INNER, R_DQUOTE)   ; "..."

; === Character Literals ===
R_CHAR_BACKSLASH = TERMINAL "\\"
R_CHAR_NEWLINE   = TERMINAL "newline"
R_CHAR_TAB_NAME  = TERMINAL "tab"
R_CHAR_SPACE_NAME= TERMINAL "space"
R_CHAR_RETURN    = TERMINAL "return"
R_CHAR_NAMED   = ALT(R_CHAR_NEWLINE, R_CHAR_TAB_NAME, R_CHAR_SPACE_NAME, R_CHAR_RETURN)
R_CHAR_SIMPLE  = ANY                                       ; any single char
R_CHAR_LITERAL = SEQ(R_CHAR_BACKSLASH, R_CHAR_SIMPLE)      ; \c

; === Delimiters ===
R_LPAREN       = TERMINAL "("
R_RPAREN       = TERMINAL ")"
R_LBRACKET     = TERMINAL "["
R_RBRACKET     = TERMINAL "]"
R_LBRACE       = TERMINAL "{"
R_RBRACE       = TERMINAL "}"
R_HASHBRACE    = TERMINAL "#{"
R_CARET        = TERMINAL "^"
R_DOTDOT       = TERMINAL ".."
R_COLONWHEN    = TERMINAL ":when"

; === Compound Forms ===
R_LIST_INNER   = ALT(R_EXPR, R_EPSILON)                    ; recursive via action
R_LIST         = SEQ(R_LPAREN, R_SKIP, R_LIST_INNER, R_SKIP, R_RPAREN)

R_SLOT_INNER   = ALT(R_EXPR, R_EPSILON)
R_SLOT         = SEQ(R_LBRACKET, R_SKIP, R_SLOT_INNER, R_SKIP, R_RBRACKET)

R_TYPE_INNER   = ALT(R_EXPR, R_EPSILON)
R_TYPE         = SEQ(R_LBRACE, R_SKIP, R_TYPE_INNER, R_SKIP, R_RBRACE)

R_DICT_INNER   = ALT(R_EXPR, R_EPSILON)
R_DICT         = SEQ(R_HASHBRACE, R_SKIP, R_DICT_INNER, R_SKIP, R_RBRACE)

; === Metadata ===
R_META_KEY     = SEQ(R_CARET, R_COLON_SYM)                 ; ^:key
R_META         = SEQ(R_CARET, R_COLON_SYM)

; === Special Forms ===
R_GUARD        = SEQ(R_COLONWHEN, R_SKIP, R_EXPR)          ; :when <condition>
R_SPREAD       = SEQ(R_DOTDOT, R_SKIP, R_SYM)              ; .. rest

; === Quote/Quasiquote ===
R_QUOTE_CHAR   = TERMINAL "'"
R_QUASIQUOTE_CHAR = TERMINAL "`"
R_UNQUOTE_CHAR = TERMINAL ","
R_UNQUOTE_SPLICE = TERMINAL ",@"                           ; must be before ,
R_QUOTED       = ALT(R_UNQUOTE_SPLICE, R_QUOTE_CHAR,       ; quote prefix
                     R_QUASIQUOTE_CHAR, R_UNQUOTE_CHAR)    ; semantic action reads expr

; === Path Expressions ===
R_PATH_SEGMENT = ALT(R_SYM, R_INT)                         ; sym / int
R_PATH_TAIL_ITEM = SEQ(R_DOT, R_PATH_SEGMENT)              ; . segment
R_PATH_TAIL    = POS(R_PATH_TAIL_ITEM)                     ; (.segment)+
R_PATH_ROOT    = ALT(R_SYM, R_INT)                         ; start of path
R_PATH         = SEQ(R_PATH_ROOT, R_PATH_TAIL)             ; foo.bar.baz

; === Set Literal ===
R_HASHSET      = TERMINAL "#set"
R_SET          = SEQ(R_HASHSET, R_LBRACE, R_SKIP, R_SLOT_INNER, R_SKIP, R_RBRACE)

; === Named Characters (with hash prefix) ===
R_HASH         = TERMINAL "#"
R_NAMED_CHAR   = SEQ(R_HASH, R_CHAR_BACKSLASH, R_SYM_CHAR) ; #\newline, #\x41

; === Format Strings ===
R_HASH_FMT     = TERMINAL "#fmt"
R_FMT_STRING   = SEQ(R_HASH_FMT, R_STRING)                 ; #fmt"..."
R_HASH_CLF     = TERMINAL "#clf"
R_CLF_STRING   = SEQ(R_HASH_CLF, R_STRING)                 ; #clf"..."

; === Hash-val Reader ===
R_HASH_VAL     = SEQ(R_HASH, R_SKIP, R_ATOM)               ; #val 42

; === Kind Splice ===
R_HASHKIND     = TERMINAL "#kind"
R_KIND_SPLICE  = SEQ(R_LBRACE, R_SKIP, R_HASHKIND, R_SKIP, R_EXPR, R_RBRACE)

; === Atom ===
R_ATOM         = ALT(R_ANY_FLOAT, R_SIGNED_INT, R_INT, R_STRING, R_COLON_SYM, R_SYM)

; === Expression (ALT order = priority) ===
R_EXPR         = ALT(R_FMT_STRING, R_CLF_STRING,           ; #fmt"...", #clf"..."
                     R_SET,                                 ; #set{...}
                     R_NAMED_CHAR,                          ; #\newline
                     R_KIND_SPLICE,                         ; {#kind expr}
                     R_DICT,                                ; #{...}
                     R_QUOTED,                              ; 'x, `x, ,x, ,@x
                     R_PATH,                                ; foo.bar.baz
                     R_LIST, R_SLOT, R_TYPE,                ; compound forms
                     R_META,                                ; metadata ^:key
                     R_GUARD,                               ; guard :when
                     R_SPREAD,                              ; spread ..
                     R_ANY_FLOAT, R_SIGNED_INT, R_INT, R_STRING,  ; literals
                     R_COLON_SYM, R_SYM)                    ; symbols

; === Program ===
R_PROGRAM_INNER = ALT(R_EXPR, R_EPSILON)
R_PROGRAM       = SEQ(R_SKIP, R_PROGRAM_INNER)
```

### C Implementation Pattern

In the actual C code, rules are defined as:

```c
/* TERMINAL - literal string */
g_omni_rules[R_LPAREN] = (PikaRule){ PIKA_TERMINAL, .data.str = "(" };

/* RANGE - character range */
g_omni_rules[R_DIGIT] = (PikaRule){ PIKA_RANGE, .data.range = {'0', '9'} };

/* ANY - matches any single character */
g_omni_rules[R_CHAR_SIMPLE] = (PikaRule){ PIKA_ANY };

/* SEQ - sequence of rules */
g_omni_rules[R_LIST] = (PikaRule){
    PIKA_SEQ,
    .data.children = { rule_ids(5, R_LPAREN, R_SKIP, R_LIST_INNER, R_SKIP, R_RPAREN), 5 },
    .action = act_list
};

/* ALT - prioritized choice */
g_omni_rules[R_ALPHA] = (PikaRule){
    PIKA_ALT,
    .data.children = { rule_ids(2, R_ALPHA_LOWER, R_ALPHA_UPPER), 2 }
};

/* REP - zero or more */
g_omni_rules[R_SKIP] = (PikaRule){
    PIKA_REP,
    .data.children = { rule_ids(1, R_WS_OR_COMMENT), 1 }
};

/* POS - one or more */
g_omni_rules[R_INT] = (PikaRule){
    PIKA_POS,
    .data.children = { rule_ids(1, R_DIGIT), 1 },
    .action = act_int
};

/* OPT - optional */
g_omni_rules[R_SIGN] = (PikaRule){
    PIKA_OPT,
    .data.children = { rule_ids(1, R_SYM_MINUS), 1 }
};
```

### Key Improvements in HVM4 Version

1. **Complete Symbol Character Set**: Original only supported `a-z`, now supports `A-Z` and all operator characters (`+`, `-`, `*`, `/`, `=`, `<`, `>`, `!`, `?`, `_`, `@`, `%`, `&`)

2. **Comments**: Full `;` to end-of-line comment support with `R_SKIP` consuming whitespace and comments together

3. **Enhanced Number Parsing**:
   - Signed integers: `+123`, `-456`
   - Multiple float forms: `3.14` (full), `.5` (leading dot), `3.` (trailing dot)
   - Signed floats: `-2.5`, `+.5`, `-3.`

4. **String Literals**: Full escape sequence support (`\n`, `\t`, `\r`, `\"`, `\\`)

5. **Character Literals**: Named characters (`#\newline`, `#\tab`, `#\space`, `#\return`, `#\xNN`)

6. **Colon-Quoted Symbols**: `:name` correctly desugars to `(quote name)`

7. **Quote/Quasiquote**:
   - Quote: `'expr`
   - Quasiquote: `` `expr ``
   - Unquote: `,expr`
   - Unquote-splicing: `,@expr`

8. **Path Expressions**: `foo.bar.baz` for member access and module paths

9. **Set Literals**: `#set{1 2 3}` for set collections

10. **Format Strings**: `#fmt"..."` and `#clf"..."` for formatted string interpolation

11. **Hash-val Reader**: `#val 42` for value-to-type conversion

12. **Kind Splice**: `{#kind expr}` for type-level splicing

13. **Metadata**: `^:key` syntax for metadata attachment

14. **Pattern Matching Support**:
    - `R_GUARD`: `:when <expr>` guard conditions
    - `R_SPREAD`: `.. rest` for rest patterns in destructuring

15. **de Bruijn Indexing**: Built-in binding stack for variable resolution during parsing

---

## See Also

- `docs/QUICK_REFERENCE.md` - Complete language reference
- `docs/TYPE_SYSTEM_DESIGN.md` - Type system details
- `TODO.md` - Design Decisions section for rationale
