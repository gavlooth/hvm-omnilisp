# Type System Guide

OmniLisp uses **Gradual Multiple Dispatch** - a hybrid type system that combines:
- Lisp's runtime flexibility
- Julia-style multiple dispatch
- Optional static type checking

Add types incrementally. Get more safety as you add them.

## Type Annotation Basics

### The Kind Domain `{}`

Types live in the Kind domain, marked with curly braces:

```lisp
;; Type annotation on a value
(define x {Int} 42)

;; Type annotation on a parameter
(define add [x {Int}] [y {Int}] {Int}
  (+ x y))
```

### Type Annotation Syntax

| Context | Syntax | Example |
|---------|--------|---------|
| Value | `(define name {Type} value)` | `(define age {Int} 25)` |
| Parameter | `[param {Type}]` | `[x {Int}]` |
| Return type | `{Type}` after params | `(define f [x] {Int} ...)` |

### Gradual Typing

Types are optional. Start untyped, add types as you refine:

```lisp
;; Untyped (any type accepted)
(define process [x] x)

;; Partially typed
(define transform [x {Number}] [y] (+ x (coerce y)))

;; Fully typed
(define multiply [x {Int}] [y {Int}] {Int}
  (* x y))
```

## Built-in Types

### Primitive Types

| Type | Description | Examples |
|------|-------------|----------|
| `{Int}` | Integer | `42`, `-17` |
| `{Float}` | Floating point | `3.14`, `-0.5` |
| `{String}` | Text string | `"hello"` |
| `{Bool}` | Boolean | `true`, `false` |
| `{Char}` | Single character | `#\a`, `#\newline` |
| `{Symbol}` | Symbol | `'foo`, `'bar` |

### Collection Types

| Type | Description | Example |
|------|-------------|---------|
| `{List}` | Linked list | `'(1 2 3)` |
| `{Array}` | Indexed array | `[1 2 3]` |
| `{Dict}` | Key-value map | `#{"a" 1}` |

### Special Types

| Type | Description |
|------|-------------|
| `{Any}` | Universal supertype (accepts anything) |
| `{Nothing}` | Singleton type with value `nothing` (also used for "no return value") |

## Defining Custom Types

### Product Types (Structs)

Define types with fields using slot syntax:

```lisp
;; Simple struct
(define Point [x {Float}] [y {Float}])

;; Creates:
;;   - Constructor: (Point 3.0 4.0)
;;   - Accessors: (Point-x p), (Point-y p)

;; Usage
(define p (Point 3.0 4.0))
(Point-x p)  ;; -> 3.0
(Point-y p)  ;; -> 4.0
```

### With Parent Type

```lisp
;; Shape is the parent type
(define Shape)

;; Circle inherits from Shape
(define ^:parent {Shape} Circle
  [center {Point}]
  [radius {Float}])

;; Rectangle inherits from Shape
(define ^:parent {Shape} Rectangle
  [corner {Point}]
  [width {Float}]
  [height {Float}])
```

### Sum Types (Enums/Variants)

Define variants as bare symbols or constructors:

```lisp
;; Simple enum
(define Color
  Red
  Green
  Blue)

;; Usage
Color.Red
Color.Green

;; ADT with data
(define Option [T]
  Nothing
  (Some [value {T}]))

;; Usage
Nothing
(Some 42)
(Some "hello")
```

### Complex ADTs

```lisp
;; Expression tree
(define Expr
  (Lit [value {Int}])
  (Add [left {Expr}] [right {Expr}])
  (Mul [left {Expr}] [right {Expr}])
  (Neg [inner {Expr}]))

;; Usage
(Lit 5)
(Add (Lit 2) (Lit 3))
(Mul (Lit 4) (Add (Lit 1) (Lit 2)))
```

## Parametric Types

### Type Parameters

Define generic types with type parameters:

```lisp
;; Generic pair
(define Pair [T] [first {T}] [second {T}])

;; Usage
(Pair 1 2)           ;; Pair of Ints
(Pair "a" "b")       ;; Pair of Strings

;; Generic list
(define List [T]
  Nil
  (Cons [head {T}] [tail {List T}]))

;; Usage
(Cons 1 (Cons 2 (Cons 3 Nil)))
```

### Multiple Type Parameters

```lisp
;; Either type with two parameters
(define Either [L R]
  (Left [value {L}])
  (Right [value {R}]))

;; Usage
(Left "error message")      ;; Either String _
(Right 42)                   ;; Either _ Int

;; Map entry
(define Entry [K V]
  [key {K}]
  [value {V}])

(Entry "name" "Alice")       ;; Entry String String
(Entry 1 true)               ;; Entry Int Bool
```

## Union and Intersection Types

### Union Types

A union accepts any of its member types:

```lisp
;; Define a union type
(define IntOrString (union [{Int} {String}]))

;; Values can be either
(define x {IntOrString} 42)
(define y {IntOrString} "hello")

;; Use in functions
(define describe [v {union [{Int} {String}]}]
  (match v
    [n {Int}]    (str "number: " n)
    [s {String}] (str "text: " s)))
```

### Intersection Types

An intersection must satisfy all member types:

```lisp
;; Value must be both Readable and Writable
(define process [x {& [{Readable} {Writable}]}]
  (let [data (read x)]
    (write x (transform data))))
```

## Type Aliases

Create short names for complex types:

```lisp
;; Alias for common types
(deftype-alias UserId {Int})
(deftype-alias Email {String})

;; Alias for complex types
(deftype-alias Result {union [{Ok} {Err}]})
(deftype-alias Callback {fn [{Int}] {Bool}})

;; Usage
(define process-user [id {UserId}] [email {Email}]
  ...)
```

## Multiple Dispatch

### Basic Multiple Dispatch

Define multiple implementations based on argument types:

```lisp
;; Same function name, different types
(define describe [x {Int}]
  (str "integer: " x))

(define describe [x {String}]
  (str "string: " x))

(define describe [x {List}]
  (str "list of " (length x) " items"))

;; Dispatch based on runtime type
(describe 42)         ;; -> "integer: 42"
(describe "hello")    ;; -> "string: hello"
(describe '(1 2 3))   ;; -> "list of 3 items"
```

### Multi-Argument Dispatch

Dispatch on all argument types simultaneously:

```lisp
;; Two Int arguments
(define combine [x {Int}] [y {Int}]
  (+ x y))

;; Two String arguments
(define combine [x {String}] [y {String}]
  (str x y))

;; Int and String
(define combine [x {Int}] [y {String}]
  (str x ": " y))

;; String and Int
(define combine [x {String}] [y {Int}]
  (str x " (" y ")"))

;; Dispatch picks most specific match
(combine 1 2)           ;; -> 3
(combine "a" "b")       ;; -> "ab"
(combine 5 "items")     ;; -> "5: items"
(combine "count" 10)    ;; -> "count (10)"
```

### Dispatch Specificity

More specific types take precedence:

```lisp
;; From most to least specific
(define process [x {Int}]    "integer")
(define process [x {Number}] "number")
(define process [x {Any}]    "anything")

(process 42)     ;; -> "integer" (Int is most specific)
(process 3.14)   ;; -> "number" (Float matches Number)
(process "hi")   ;; -> "anything" (fallback to Any)
```

### Diagonal Dispatch

Constrain multiple parameters to the same type:

```lisp
;; x and y must be the same type T
(define ^:where [T {Number}]
  equal-add [x {T}] [y {T}] {T}
  (+ x y))

;; Both Int -> Int
(equal-add 1 2)        ;; -> 3

;; Both Float -> Float
(equal-add 1.0 2.0)    ;; -> 3.0

;; Mixed types - error
(equal-add 1 2.0)      ;; Type error: Int and Float don't unify
```

## Type Predicates

### Runtime Type Checking

```lisp
;; Check type at runtime
(type? 42 Int)         ;; -> true
(type? "hello" Int)    ;; -> false
(type? "hello" String) ;; -> true

;; Subtype checking works
(type? 42 Number)      ;; -> true (Int is subtype of Number)
(type? 42 Any)         ;; -> true (everything is Any)
```

### Type-Safe Casting

```lisp
;; Cast with runtime check
(define safe-to-int [x]
  (if (type? x Int)
      (Some x)
      Nothing))

;; Use in pattern matching
(match (safe-to-int value)
  [(Some n)] (process-int n)
  [Nothing]  (handle-non-int value))
```

## Type Inference

OmniLisp infers types when not specified:

```lisp
;; Inferred from usage
(define add [x] [y]
  (+ x y))
;; Inferred: {Number} -> {Number} -> {Number}

;; Inferred from literal
(define double [x]
  (* x 2))
;; Inferred: {Number} -> {Number}

;; Check inferred type
(type-of add)  ;; -> {fn [{Number}] {fn [{Number}] {Number}}}
```

## Variance

### Covariance (Output Position)

Covariant types preserve subtyping direction:

```lisp
;; List is covariant in T
(define ^:covar {List} [T]
  Nil
  (Cons [head {T}] [tail {List T}]))

;; List Int is subtype of List Number
;; Because Int is subtype of Number
```

### Invariance (Input/Output Position)

Invariant types require exact match:

```lisp
;; Mutable array is invariant
(define ^:invariant {MutArray} [T]
  [data {T}]
  [capacity {Int}])

;; MutArray Int is NOT subtype of MutArray Number
;; To prevent unsafe writes
```

## Handling Ambiguity

When multiple dispatch is ambiguous, OmniLisp provides options:

### Add a Fallback

```lisp
;; Ambiguous for Vector Number
(define foo [x {Vector Int}] 1)
(define foo [x {Vector Float}] 2)
;; What about Vector Number?

;; Solution: add fallback
(define foo [x {Vector Number}] 3)
;; Now Vector Number has explicit handling
```

### Use Priority

```lisp
;; Explicit priority when ambiguous
(define ^:priority [Int Float] bar
  [x {Int}] 1)

(define bar [x {Float}] 2)
```

## Best Practices

### 1. Start Untyped, Add Types Gradually

```lisp
;; Phase 1: Prototype untyped
(define process [data]
  (transform data))

;; Phase 2: Add types to public API
(define process [data {UserData}] {Result}
  (transform data))

;; Phase 3: Add types internally for safety
(define transform [data {UserData}] {Result}
  (let [validated {ValidData} (validate data)]
    (compute validated)))
```

### 2. Use Types for Documentation

```lisp
;; Types make intent clear
(define send-email
  [to {Email}]
  [subject {String}]
  [body {Html}]
  {Result})
```

### 3. Leverage Dispatch for Polymorphism

```lisp
;; Instead of conditionals
(define area [shape {Shape}]
  (match shape
    [(Circle r)]     (* 3.14 r r)
    [(Rectangle w h)] (* w h)))

;; Use dispatch
(define area [c {Circle}]
  (let [r (Circle-radius c)]
    (* 3.14 r r)))

(define area [r {Rectangle}]
  (* (Rectangle-width r) (Rectangle-height r)))
```

### 4. Define Type Aliases for Clarity

```lisp
;; Instead of
(define process [id {Int}] [email {String}] ...)

;; Use aliases
(deftype-alias UserId {Int})
(deftype-alias Email {String})
(define process [id {UserId}] [email {Email}] ...)
```

## Summary

| Feature | Syntax | Example |
|---------|--------|---------|
| Type annotation | `{Type}` | `{Int}`, `{String}` |
| Typed parameter | `[param {Type}]` | `[x {Int}]` |
| Return type | `{Type}` after params | `(define f [x] {Int} ...)` |
| Product type | `(define Name [field {Type}] ...)` | `(define Point [x {Int}] [y {Int}])` |
| Sum type | `(define Name Variant1 Variant2 ...)` | `(define Color Red Green Blue)` |
| Parametric | `(define Name [T] ...)` | `(define List [T] ...)` |
| Union | `(union [types...])` | `(union [{Int} {String}])` |
| Intersection | `{& [types...]}` | `{& [{A} {B}]}` |
| Type alias | `(deftype-alias Name {Type})` | `(deftype-alias Id {Int})` |
| Parent type | `^:parent {Parent}` | `^:parent {Shape} Circle` |
| Where clause | `^:where [T {Bound}]` | `^:where [T {Number}]` |

## Further Reading

- [Pattern Matching Guide](PATTERN_MATCHING_GUIDE.md) - Pattern matching with types
- [API Reference](API_REFERENCE.md) - Type system functions
- [Quick Reference](QUICK_REFERENCE.md) - Language overview
- [TYPE_SYSTEM_DESIGN.md](TYPE_SYSTEM_DESIGN.md) - Design rationale
