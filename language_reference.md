# OmniLisp Language Reference

This is the authoritative reference for the OmniLisp language, its "Character Calculus" semantics, and its optionally-typed execution model.

---

## 1. The Character Calculus (Delimiters)

OmniLisp uses a fixed semantic "Charge" for every delimiter to ensure absolute consistency across the value and type domains.

| Character | Domain | Purpose |
| :--- | :--- | :--- |
| **`()`** | **Flow** | Execution logic, function calls, and **Value Constructors**. |
| **`[]`** | **Slot** | Argument vectors, `let` bindings, arrays, and **Type Parameters**. |
| **`{}`** | **Kind** | **Type Annotations** and the Blueprint domain. |
| **`.`**  | **Path** | Data location and navigation. Bridges Flow and Slot. |
| **`^`**  | **Tag**  | Metadata and out-of-band instructions. |
| **`#`**  | **Sign** | Reader dispatch for literal structures (Dicts). |

---

## 2. Bindings & Function Definitions

Functions use Slot `[]` syntax for parameters, with **optional** type annotations in Kind `{}`.

### 2.1 Function Definition Syntax

**Two forms are supported:**

#### Traditional Shorthand (Ergonomic Default)
```lisp
;; No types - concise for prototyping
(define add x y
  (+ x y))

;; Single parameter
(define square x
  (* x x))

;; Equivalent to:
(define add [x] [y]
  (+ x y))
```

#### Slot Syntax (Explicit/Typed)
```lisp
;; All parameters typed
(define add [x {Int}] [y {Int}] {Int}
  (+ x y))

;; Mixed - some typed, some not
(define map [fn] [xs {List}] {List}
  (fn xs))

;; All parameters in Slots, types optional
(define process [x] [y {Int}] [z {Float}]
  (body))
```

**Syntax Pattern:**
```lisp
(define function-name
  [param1 {Type}?]    ;; Slot with optional type
  param2              ;; Shorthand: param without Slot = untyped
  [param3 {Type}?]
  {ReturnType}?        ;; Optional return type
  body)
```

### 2.2 Diagonal Dispatch & Constraints (`^:where`)
To enforce that multiple arguments share the same type or to restrict generics.

```lisp
;; x and y MUST be the same T, where T is a Number
(define ^:where [T {Number}]
  [x {T}] [y {T}] {T}
  (+ x y))
```

### 2.3 Lambda Shorthands (`fn` and `λ`)
The symbols `fn` and `λ` use the same Slot pattern.

```lisp
;; Untyped parameters (shorthand)
(fn x y
  (+ x y))

;; Single parameter
(fn [x] (* x x))
(λ [x] (* x x))

;; Typed parameters
(fn [x {Int}] [y {Int}] {Int}
  (+ x y))

;; Mixed types
(fn [x] [y {String}] {String}
  (concat x y))
```

**Consistency Note:** Lambdas follow the same pattern as `define` - Slots `[]` for parameters, optional types in `{}`.

### 2.4 Local Bindings (`let`)
Let bindings use Slot `[]` syntax with optional types: `[name {Type}? value]`

```lisp
;; Untyped bindings (shorthand)
(let [x 10] [y 20]
  (+ x y))

;; Typed bindings
(let [x {Int} 10] [y {Int} 20]
  (+ x y))

;; Mixed - some typed, some not
(let [x 10] [y {Int} 20]
  (+ x y))

;; Sequential let (each binding sees previous ones)
(let ^:seq [x 1] [y (+ x 1)]
  y)
```

**Consistency:** Let bindings follow the same `[name {Type}?]` pattern as function parameters.

### 2.4.1 Destructuring in `let`

`let` supports minimal destructuring patterns for sequences and dictionaries/plists.

#### Sequence Destructuring
Positional destructuring works for **both lists and arrays**:

```lisp
;; Destructure a sequence into x, y, z
(let [[x y z] my-seq]
  (+ x y z))
;; my-seq can be: (list 1 2 3) OR [1 2 3] - same behavior!
```

#### Splicing
Capture remaining elements with `..`:

```lisp
;; Bind first two elements, capture rest
(let [[x y .. rest] my-seq]
  (append (list x y) rest))
;; x = first element, y = second, rest = remaining elements
```

#### Dictionary/Plist Destructuring
Pull symbols by name from dictionaries or property lists:

```lisp
;; Pull symbols :x and :y from dictionary/plist
(let [(:x :y) my-dict]
  (+ x y))
;; Equivalent to: x = my-dict[:x], y = my-dict[:y]
```

**Note:** In OmniLisp, `:x` is pure reader sugar for `'x` (no separate keyword type). The destructuring pattern `(:x :y)` extracts values associated with those symbols. For canonicalization/pretty-printing, prefer the `'x` form.

### 2.5 Type Enforcement
Every variable has an associated Kind. If no Kind is specified, it defaults to **`Any`**.
*   **Assignments:** `define`, `let`, and `set!` validate values against the Kind blueprint.
*   **Application:** `omni_apply` validates arguments against parameter Kinds and the result against the return Kind.

### 2.6 Multiple Dispatch & Multi-arity
OmniLisp functions support **Multiple Dispatch**, allowing a single name to refer to multiple implementations (methods). Dispatch is determined by:
1.  **Arity:** The number of arguments provided.
2.  **Specificity:** The Kind of each argument (Most specific match wins).

```lisp
;; 1. Multi-arity (different argument counts)
(define area 0)                      ; 0 args
(define area [r] (* 3.14 (* r r)))   ; 1 arg

;; 2. Type Specialization (Multiple Dispatch)
(define describe [x {Int}] "An integer")
(define describe [x {String}] "A string")

;; 3. Combined Dispatch
(define add [x {Int}] [y {Int}] (+ x y))
(define add [x {String}] [y {String}] (string-append x y))
```

#### Type Coercion Pattern

Multiple dispatch enables elegant **type coercion** functions that convert between types:

```lisp
;; The `list` coercer uses multiple dispatch for each input type:
(define list [x {List}] {List} x)                     ; identity
(define list [x {Array}] {List} (array->list x))      ; array → list
(define list [x {Iterator}] {List} (realize x))       ; realize iterator
(define list [x {Range}] {List} (realize x))          ; realize range
(define list [x {Dict}] {List} (dict-entries x))      ; dict → list of pairs

;; Similarly for `array`:
(define array [x {Array}] {Array} x)                  ; identity
(define array [x {List}] {Array} (list->array x))     ; list → array
(define array [x {Iterator}] {Array} (list->array (realize x)))

;; Usage:
(list [1 2 3])        ; Array → List: (1 2 3)
(array '(a b c))      ; List → Array: [a b c]
(list (range 5))      ; Iterator → List: (0 1 2 3 4)
```

This pattern provides:
- **Uniform interface**: Same function name for all conversions
- **Type safety**: Dispatch ensures correct conversion method
- **Efficiency**: Identity returns input unchanged (no copying)

### 2.7 Pipe Operator (`|>`)

The pipe operator threads a value through a series of functions. Each function receives the result of the previous one as its argument.

#### Basic Syntax

```lisp
;; Prefix pipe: (|> value fn1 fn2 ...)
(|> 5 inc)                  ; → (inc 5) → 6

;; Chained functions
(|> 5 inc square)           ; → (square (inc 5)) → 36

;; With lambda
(|> 10 (lambda [x] (+ x 1))) ; → 11
```

#### Desugaring

The pipe operator desugars at parse time to nested function application:

```lisp
;; This:
(|> 5 inc square double)

;; Becomes:
(double (square (inc 5)))
```

### 2.8 Function Combinators

OmniLisp provides combinators for argument manipulation, useful with pipes and higher-order functions.

#### `curry` - Convert to Curried Form

Converts a multi-argument function to curried form (one argument at a time):

```lisp
(define add [x y] (+ x y))    ; 2-arg function

(curry add)                    ; → λx.λy.(+ x y)
((curry add) 5)                ; → λy.(+ 5 y)
(((curry add) 5) 3)            ; → 8

;; Useful in pipes
(|> 5 (curry +) 10)            ; → 15
```

For multi-arity functions, specify which arity to curry:

```lisp
(define f
  [x]       x                  ; 1-arg version
  [x y]     (+ x y))           ; 2-arg version

(curry f 1)                    ; curry the 1-arg version
(curry f 2)                    ; curry the 2-arg version
((curry f 2) 5)                ; → λy.(+ 5 y)
```

#### `flip` - Swap First Two Arguments

Swaps the first two arguments of a function:

```lisp
((flip -) 5 10)               ; → (- 10 5) → 5
((flip /) 2 10)               ; → (/ 10 2) → 5

;; Useful when piped value should be second argument
(|> 5 (flip -) 10)            ; → (- 10 5) → 5
(|> 2 (flip /) 10)            ; → (/ 10 2) → 5
```

#### `rotate` - Cycle Arguments Left

Rotates arguments left: first argument moves to end:

```lisp
;; For f(a, b, c), (rotate f) gives f(b, c, a)
((rotate f) a b c)            ; → (f b c a)

;; Useful when piped value should be last argument
(|> 5 (rotate /) 100 2)       ; → (/ 100 2 5) - if / took 3 args
```

#### `comp` - Function Composition

Composes functions right-to-left:

```lisp
((comp square inc) 5)         ; → (square (inc 5)) → 36
((comp double square inc) 3)  ; → (double (square (inc 3))) → 32
```

**Summary:**

| Combinator | Effect | Example |
|------------|--------|---------|
| `(curry f)` | Multi-arg → curried | `((curry f) a b)` = `((f a) b)` |
| `(curry f n)` | Curry specific arity | `(curry f 2)` curries 2-arg version |
| `(flip f)` | Swap first two args | `((flip f) a b)` = `(f b a)` |
| `(rotate f)` | First arg to end | `((rotate f) a b c)` = `(f b c a)` |
| `(comp f g ...)` | Right-to-left compose | `((comp f g) x)` = `(f (g x))` |

---

## 3. The Type System (Kind Domain)

OmniLisp adopts a **Julia-compatible Type System** defined via the Uniform Definition syntax `(define {Kind ...} ...)`.

### 3.1 Type Definitions

#### Abstract Types
Abstract types cannot be instantiated. They serve as nodes in the type graph.

```lisp
;; Abstract type 'Number'
(define {abstract Number})

;; Abstract type with parent (using metadata)
(define ^:parent {Number} {abstract Integer} [])
```

#### Primitive Types
Types with a fixed bit-width representation, usually provided by the host/compiler.

```lisp
;; Define a 64-bit primitive type
(define ^:parent {Integer} {primitive Int64} [64])
```

#### Composite Types (Structs)
Product types that hold named fields. Immutable by default.

```lisp
;; Immutable Struct (with parent type)
(define ^:parent {Any} {struct Point}
  [x {Int}]
  [y {Int}])

;; Mutable Struct
(define {mutable-struct Person}
  [name {String}]
  [age  {Int}])
```

#### Union Types
Represents a value that can be one of several types. Unions are **Flow constructors** that return a Kind.

```lisp
;; Create a union type (Flow constructor)
(union [{Int} {String}])

;; Type alias: name the union
(define {IntOrString}
  (union [{Int} {String}]))

;; Empty union is the Bottom type (no values possible)
;; Equivalent to Julia's Union{}
(define {Bottom}
  (union []))  ; No values can have this type

;; Using union in annotations
(define process [val {(union [{Int} {String}])}]
  (match val
    [x {Int} (* x 2)]
    [s {String} (string-length s)]))
```


**Note:** The empty union `(union [])` is the **bottom type** (called `Union{}` in Julia). No value can have this type, making it useful for type theory and proving impossibility.

#### Parametric Types
Types that take type parameters (Generics).

```lisp
;; Parametric Struct
(define {struct Node [T]}
  [value {T}]
  [next  {Option {Node T}}])
```

#### Variance (Orthogonal Types)
Variance annotations hint how parametric types relate in the subtyping lattice. This follows Julia's type system conventions.

```lisp
;; Covariant - subtyping follows parameter
(define {struct [^:covariant T]} {Tuple}
  [elements {T}])
;; (Tuple {Integer}) ⊑ (Tuple {Number}) because Integer ⊑ Number

;; Invariant - orthogonal types (no subtyping relationship)
(define {struct [^:invariant T]} {Vector}
  [data {T}])
;; (Vector {Integer}) and (Vector {Number}) are orthogonal
;; Neither is a subtype of the other

;; Contravariant - subtyping is reversed
(define {abstract [^:contravariant T]} {Consumer}
  [accept [self {Self}] [val {T}]])
;; (Consumer {Number}) ⊑ (Consumer {Integer})
```

**Variance in the Type Lattice:**
- **Covariant (`^:covariant`)**: If `A ⊑ B`, then `F{A} ⊑ F{B}`. Subtyping follows the parameter.
- **Invariant (`^:invariant`)**: `F{A}` and `F{B}` are **orthogonal** - neither is a subtype of the other, even if `A ⊑ B`.
- **Contravariant (`^:contravariant`)**: If `A ⊑ B`, then `F{B} ⊑ F{A}`. Subtyping is reversed.

**Note:** Invariant types are orthogonal in the type lattice. This is a property of the type relationships, not a statement about mutability.

### 3.2 Annotations vs. Constructors
*   **Annotation `{}`**: Used in definitions to restrict a slot.
*   **Constructor `()`**: Used at the value level to create instances.

```lisp
(define x {Int} 5)           ; {Int} is an annotation
(define p (Point 10 20))     ; (Point) is a constructor (Flow)
```

### 3.3 First-Class Kinds
Base types (`Int`, `String`, `Array`, `Any`, etc.) are physical Kind objects stored in the global environment. They can be manipulated as values.

```lisp
(define my-type Int)
(type? 10 my-type)           ; => true
```

### 3.4 Subtyping
OmniLisp uses a nominative subtype hierarchy. **`Any`** is the universal supertype.

Subtype relationships are declared using `^:parent` metadata:

```lisp
;; Int is a subtype of Number
(define ^:parent {Number} {abstract Int} [])

;; Number is a subtype of Any (implicit, as Any is root)
(define {abstract Number} [])
```

*   **`Any`** is the universal supertype (root of the type hierarchy)
*   **`Union{}`** (empty union) is the universal subtype (bottom type, has no values)
*   **`Nothing`** is a concrete singleton type with the single value `nothing` (similar to Julia)

### 3.5 Value Types (Singleton Types)

Value types (also called singleton types or literal types) constrain a type to a single specific value. They are created by placing a literal value inside braces `{}`:

```lisp
;; Value type syntax: {literal} = #val literal
{3}           ; Type that only contains the value 3
{true}        ; Type that only contains true
{"hello"}     ; Type that only contains "hello"
{#\a}         ; Type that only contains character 'a'
{1.5}         ; Type that only contains 1.5
```

**Equivalence:** `{literal}` is syntactic sugar for `#val literal`:
```lisp
{42}          ; equivalent to #val 42
{false}       ; equivalent to #val false
```

**Use Cases:**

```lisp
;; Exact value constraints
(define pi {3.14159} 3.14159)           ; pi can ONLY be this value

;; Type-level programming
(define status {String} "active")        ; any string
(define status {"active"} "active")      ; ONLY "active" allowed

;; Pattern matching on literal types
(match x
  [n {0}] "zero"                         ; matches only 0
  [n {1}] "one"                          ; matches only 1
  [n {Int}] "other integer")             ; matches any other int

;; Function constraints
(define must-be-positive [n {Int} & (> n 0)] n)
```

**Type Relationships:**
- `{3}` is a subtype of `{Int}` (value types are subtypes of their containing type)
- `{true}` is a subtype of `{Bool}`
- Two value types are equal only if their values are equal

### 3.6 Types as Values (First-Class Kinds)

In OmniLisp, types themselves are values that can be passed around, stored, and referenced. This leads to three distinct patterns in definitions:

#### Pattern 1: Type Inheritance (Metadata)
Use `^:parent` to declare that a type is a subtype of another:
```lisp
;; Integer IS-A Number (inheritance)
(define ^:parent {Number} {abstract Integer} [])
```

#### Pattern 2: Type Annotation (Kind)
Use `{}` to annotate that a value must conform to a type:
```lisp
;; my-number must be an Int (annotation)
(define my-number {Int} 42)

;; Function parameter with type constraint
(define process [x {String}] {Int}
  (string-length x))
```

#### Pattern 3: Type Objects as Values
A type object can be the **value** of a definition (not annotated, not metadata):
```lisp
;; IntType is a variable whose VALUE is the Int type object
(define IntType Int)

;; AnimalType holds the Animal type object
(define AnimalType Animal)

;; Using type objects for dispatch
(define check [value {Any} type-check {(Type {Int})}]
  (if (type? value type-check)
      "is an integer"
      "not an integer"))
```

**Key Distinction:**
- `^:parent {Parent}` = Metadata (declares inheritance)
- `{Type}` = Annotation (constrains a value)
- `Type` (no braces) = Value (the type object itself)

**Metadata Rule:** If the same metadata key appears multiple times (e.g. `^:parent` twice), the **last occurrence wins**.

### 3.6 Type Predicates

OmniLisp provides predicates for checking types at runtime.

#### `type?` Predicate
Check if a value is of a specific type:

```lisp
;; Check against Kind objects
(type? 42 Int)         ; => true
(type? "hello" Int)    ; => false
(type? "hello" String) ; => true
(type? [1 2 3] Array)  ; => true

;; Works with subtype relationships
(type? 5 Number)       ; => true (Int is a subtype of Number)
(type? 5 Any)          ; => true (everything is a subtype of Any)

;; Using type objects as values
(define IntType Int)
(type? 42 IntType)     ; => true
```

**Note:** The `type?` predicate uses the runtime type registry for subtype checking. Values of a subtype will return `true` when checked against their parent types.

## 4. Sequences & Iterators

### 4.1 Type Coercion (Multiple Dispatch)

OmniLisp provides **multi-dispatch coercers** to convert between sequence types:

```lisp
;; list: Coerce any sequence to List
(list [1 2 3])           ; Array → List: (1 2 3)
(list (range 5))         ; Iterator → List: (0 1 2 3 4)
(list '(a b c))          ; List → List: identity

;; array: Coerce any sequence to Array
(array '(1 2 3))         ; List → Array: [1 2 3]
(array (range 5))        ; Iterator → Array: [0 1 2 3 4]
(array [a b c])          ; Array → Array: identity

;; dict: Coerce sequence of pairs to Dict
(dict '((a 1) (b 2)))    ; List → Dict: #{:a 1 :b 2}
(dict #{:x 10})          ; Dict → Dict: identity
```

These coercers use **multiple dispatch** to handle each input type efficiently:
- **Identity**: Same-type coercion returns the input unchanged
- **Conversion**: Cross-type coercion performs efficient conversion
- **Realization**: Lazy iterators are fully realized into the target type

### 4.2 Lazy Iterators
Core sequence functions are lazy when given iterators and return `#Iter` objects.
*   `(range n)`: Returns an iterator from 0 to n-1.
*   `(map f iter)`: Returns a lazy transformed iterator (when given iterator).
*   `(filter pred iter)`: Returns a lazy filtered iterator (when given iterator).

### 4.3 Parallel-by-Default Semantics

When operating on **eager types** (List, Array, Dict), sequence operations are **parallel by default**:

```lisp
;; These use fork2 for automatic parallelism:
(map square '(1 2 3 4 5 6 7 8))      ; Parallel map over list
(filter even? '(1 2 3 4 5 6 7 8))    ; Parallel filter
(foldl + 0 '(1 2 3 4 5 6 7 8))       ; Parallel tree reduction
```

**How it works:**
- Lists are split in half recursively
- Each half is processed in parallel using `fork2`
- Results are combined (append for map/filter, apply op for fold)

**When to use sequential:**
- Non-associative operations in fold
- When ordering/side-effects matter
- Very small lists (parallelism overhead)

```lisp
;; Sequential versions using ^:seq metadata (preferred):
(map ^:seq f xs)           ; Sequential map
(filter ^:seq pred xs)     ; Sequential filter
(foldl ^:seq f acc xs)     ; Sequential left fold

;; Equivalent internal function calls (also available):
(map_seq f xs)             ; Sequential map
(filter_seq pred xs)       ; Sequential filter
(foldl_seq f acc xs)       ; Sequential left fold
```

The `^:seq` metadata provides a clean, consistent syntax for opting into sequential processing. It mirrors the `^:seq` syntax used for sequential `let` bindings:

```lisp
;; Sequential let (each binding sees previous ones)
(let ^:seq [x 1] [y (+ x 1)] y)  ; → 2

;; Sequential map (process elements in order)
(map ^:seq println '(1 2 3))     ; prints 1, 2, 3 in order
```

**Iterator paths remain lazy:**
```lisp
;; These stay lazy (no parallelism):
(map square (range 1000000))   ; Returns lazy iterator
(filter even? (range 1000000)) ; Returns lazy iterator

;; Realize when needed:
(list (map square (range 10))) ; → (0 1 4 9 16 25 36 49 64 81)
```

### 4.4 Performance Notes

| Input Type | Behavior | Parallelism |
|------------|----------|-------------|
| List (`#CON`) | Eager | ✅ Parallel via `fork2` |
| Array (`#Arr`) | Eager | ✅ Parallel (uses list parallelism) |
| Dict (`#Dict`) | Eager | ✅ Parallel (maps/filters values) |
| Iterator (`#Iter`) | Lazy | ❌ Sequential (lazy by nature) |
| Range (`#Rang`) | Lazy | ❌ Sequential (lazy by nature) |

For very small collections (< 8 elements), the overhead of parallelism may exceed benefits. The runtime optimizes this automatically via HVM4's superposition semantics.

---

## 5. Access & Mutation

### 5.1 Path Navigation
The dot `.` operator is used for nested data access.
*   `person.name` -> `(get person :name)`
*   `arr.[0]` -> `(get arr 0)`

### 5.2 Mutation
*   `set!`: Rebinds a name.
*   `put!`: Mutates a path (e.g., `(put! person.age 31)`).
*   `update`: Functionally transforms a path.

---

## 6. Control Flow & Effects

### 6.1 Algebraic Effects
OmniLisp uses **Algebraic Effects** as its primary mechanism for non-local control flow and error handling. Traditional `try/catch` is replaced by `handle/perform`.

*   **`handle`**: Establishes a handler for a specific set of effects.
*   **`perform`**: Triggers an effect, transferring control to the nearest handler.
*   **`resume`**: Resumes the suspended computation from within a handler.

```lisp
(define ^:one-shot {effect ask})

(handle
  (+ 1 (perform ask nothing))
  (ask [payload resume] (resume 42)))
;; => 43
```

### 6.2 Fibers (Lightweight Concurrency)
OmniLisp implements lightweight **Fibers** (continuations) for massive concurrency (1M+ fibers).

#### Fiber Management

| Form | Description |
|------|-------------|
| `(spawn body)` | Create and start a fiber from body expression |
| `(yield val)` | Suspend current fiber, yielding a value to mailbox |
| `(fiber-resume f val)` | Resume suspended fiber with a value |
| `(fiber-done? f)` | Check if fiber has completed |
| `(fiber-result f)` | Get final result from completed fiber |
| `(fiber-mailbox f)` | Get list of yielded values |

#### Basic Fiber Usage

```lisp
;; Create a fiber that yields multiple values
(define f (spawn
  (do
    (yield 1)
    (yield 2)
    (yield 3)
    42)))  ; Final result

;; Check fiber state
(fiber-done? f)        ; → false (suspended after first yield)

;; Resume the fiber (runs until next yield or completion)
(fiber-resume f 'continue)

;; Get all yielded values
(fiber-mailbox f)      ; → (1 2 3)

;; Get final result (only valid when fiber is done)
(fiber-result f)       ; → 42
```

#### Generator Pattern

```lisp
;; Fibonacci generator using fibers
(define (fib-gen n)
  (spawn
    (let loop [a 0] [b 1] [i 0]
      (if (< i n)
          (do
            (yield a)
            (loop b (+ a b) (+ i 1)))
          'done))))

(define fibs (fib-gen 10))
(fiber-mailbox (fiber-resume (fiber-resume fibs 0) 0))
;; → (0 1 1 2 3 5 8 13 21 34)
```

#### Communication via Effects
Fibers communicate using **algebraic effects** rather than Go-style channels. Since HVM4 has no shared mutable state, effects provide a natural way for fibers to interact with the scheduler and each other without requiring channel synchronization primitives.

### 6.3 Parallel Combinators (HVM4 Superposition)

OmniLisp provides parallel combinators that exploit HVM4's native superposition semantics for automatic parallelism. These are **prelude functions** (not special forms) that build on two primitives:

| Primitive | Description |
|-----------|-------------|
| `(fork2 a b)` | Fork two computations for parallel execution |
| `(choice xs)` | Nondeterministic choice from a list |

#### Parallel Map, Fold, Filter

These combinators split work into balanced pairs, recursively applying `fork2` for maximum parallelism. Optimal speedup occurs when operations are pure and independent.

```lisp
;; Parallel map - splits list in half, maps each half in parallel
(par-map square (range 1 1000))

;; Parallel fold - balanced tree reduction (requires associative op)
(par-fold + 0 (range 1 1000))        ;; → 499500
(par-sum (range 1 1000))             ;; Optimized parallel sum
(par-product (range 1 10))           ;; Optimized parallel product

;; Parallel filter
(par-filter even? (range 1 100))

;; Parallel predicates
(par-all? positive? numbers)         ;; Are all positive?
(par-any? zero? numbers)             ;; Is any zero?

;; Parallel zipWith
(par-zipWith + xs ys)                ;; Element-wise addition
```

**Chunked operations** for fine-grained control:

```lisp
;; Map with explicit chunk size (better for I/O-bound work)
(par-map-chunked 100 expensive-fn large-list)
```

#### Nondeterministic Choice (Amb)

The `amb` (ambiguous) operator enables nondeterministic programming. HVM4's superposition semantics explores all possible choices in parallel, returning all valid results.

```lisp
;; Choose one value nondeterministically
(amb '(1 2 3 4 5))                   ;; → one of 1..5

;; Binary choice
(amb2 'heads 'tails)                 ;; → 'heads or 'tails

;; Require a condition (prune invalid branches)
(amb-require (> x 0))                ;; Fails branch if x <= 0

;; Constraint solving example: find pairs where x + y = 10
(let [x (amb-range 1 10)]
  (let [y (amb-range 1 10)]
    (amb-require (= (+ x y) 10))
    (list x y)))
;; → all valid pairs: (1 9), (2 8), (3 7), ...
```

**Search patterns:**

```lisp
;; Find first element matching predicate
(amb-find even? numbers)

;; Find all matching elements
(amb-find-all prime? (range 2 100))

;; Choose from range
(amb-range 1 100)                    ;; → one of 1..99

;; Combinations: choose n elements from list
(amb-choose-n 2 '(a b c d))          ;; → all 2-combinations
```

**Collecting results:**

```lisp
;; Collect all valid results from nondeterministic computation
(amb-collect
  (let [x (amb '(1 2 3))]
    (amb-require (odd? x))
    (* x 2)))
;; → (2 6)  ;; 1*2 and 3*2

;; Get first successful result
(amb-first (amb-find even? numbers))
```

**Parallel search** combines `fork2` and `choice`:

```lisp
;; Explore choices in parallel
(par-amb large-search-space)

;; Parallel search with predicate
(par-search valid-solution? candidates)
```

#### When to Use

| Use Case | Recommended |
|----------|-------------|
| CPU-bound pure functions | `par-map`, `par-fold`, `par-filter` |
| Balanced tree reductions | `par-fold` with associative op |
| Search problems | `amb`, `amb-find`, `par-search` |
| Constraint solving | `amb-require`, `amb-tuple` |
| Speculative execution | `amb2`, `fork2` |
| I/O-bound work | Prefer `par-map-chunked` or fibers |

**Performance notes:**
- Parallelism is automatic but not free—avoid for very small lists
- Best speedup with pure, independent operations
- `fork2` creates superposition; HVM4 runtime decides execution strategy
- `choice` on empty list fails (use for constraint pruning)

### 6.4 Pattern Matching (`match`)

**Pattern matching is the source of truth for all control flow in OmniLisp.** The `if` form is syntactic sugar that desugars to a binary `match`.

#### Implementation Status

**✅ Currently Working:**
- `if` → `match` desugaring (all `if` expressions work)
- Binary boolean match optimization (emits branchless ternary)
- Guard syntax parsing and code generation
- Match AST construction

**⚠️ Partially Working:**
- Literal pattern matching (infrastructure in place, needs `is_pattern_match`)
- Variable pattern binding
- Simple destructuring patterns

**❌ Not Yet Implemented:**
- `is_pattern_match()` runtime function
- Full pattern matching execution
- Complex destructuring with nested patterns
- Splicing patterns (`..`)

**TODO:** See task **T-wire-pattern-match-01** for `is_pattern_match` implementation.

#### Syntax

```lisp
(match expr
  pattern1 result1
  pattern2 result2
  ...
  _ default)           ; Wildcard pattern (catch-all)
```

**Key Design Decisions:**
- **Implicit pairs:** Each `(pattern result)` pair forms a match clause
- **`_` wildcard:** Catches all unmatched patterns (should be last)
- **Flat syntax:** No grouping needed - patterns and results alternate
- **Exhaustiveness:** Compiler may warn if non-exhaustive (future feature)

#### Pattern Types

##### 1. Literal Patterns
Match exact values:

```lisp
(match x
  1 "one"
  2 "two"
  true "yes"
  false "no"
  _ "other")
```

##### 2. Variable Patterns
Bind any value to a name:

```lisp
(match value
  x (println "Got: " x))  ; x binds to value
```

##### 3. Destructuring Patterns
Decompose sequences and structures:

```lisp
;; Array/list destructuring
(match [1 2 3]
  [x y z] (+ x y z))      ; x=1, y=2, z=3

;; Nested destructuring
(match (list 1 (list 2 3))
  [1 [x y]] (+ x y))      ; x=2, y=3

;; Dictionary destructuring
(match #{:name "Alice" :age 30}
  (:name :age) (println name " is " age " years old"))
```

##### 4. Splicing Patterns
Capture remaining elements with `..`:

```lisp
(match my-list
  [x y .. rest] (cons x (cons y rest))  ; Bind first two, capture rest
  _ nothing)
```

##### 5. Type Constraints
Patterns can include type annotations using `{}`:

```lisp
(match value
  [n {Int}] (* n 2)         ; Only matches if n is an Int
  [s {String}] (string-length s)
  _ 0)
```

##### 6. Guards (`&`)
Add additional conditions to patterns using the `&` symbol:

```lisp
(match x
  [n {Int} & (> n 10)] "large"
  [n {Int} & (< n 0)] "negative"
  [n {Int}] "small or zero"
  _ "not an integer")
```

**Guard Semantics:**
- Guards are evaluated **after** the pattern matches
- If the guard is false, matching continues to the next clause
- Guards can reference bindings from the pattern
- Multiple guards can be chained: `[n {Int} & (> n 0) & (< n 100)] "in range"`

**Why `&`?**
- Preserves the `pattern result` alternation in match clauses
- Keeps guard inside the pattern specification (Slot domain `[]`)
- `&` means "and also" - pattern matches AND guard passes
- Common in other languages (Rust's `if`, Scala's `guard`, Elixir's `when`)

#### `if` as Derived Form

The `if` special form desugars to a binary match:

```lisp
;; Source
(if condition then-branch else-branch)

;; Desugars to
(match condition
  true then-branch
  false else-branch)
```

**Why this matters:**
- Single optimization pass for all control flow
- Match compiler can emit branchless code for binary boolean matches
- Consistent semantics across all branching constructs

#### Pattern Matching and Perceus Reuse

Pattern matching enables **Perceus-style in-place updates**:

```lisp
;; Can reuse x's memory for the result
(match x
  [(Some y) (consume y)]    ; consume() indicates x is no longer needed
  [(None) (default-value)])
```

When the compiler proves `x` is dead after the match, it can reuse `x`'s memory for the result (same-size optimization).

#### Examples

```lisp
;; Factorial with pattern matching
(define factorial [n {Int}] {Int}
  (match n
    0 1
    1 1
    _ (* n (factorial (- n 1)))))

;; List processing with guards
(define sum-positive [nums {List}] {Int}
  (match nums
    [& (empty? nums)] 0
    [head .. tail & (> head 0)] (+ head (sum-positive tail))
    [head .. tail] (sum-positive tail)))

;; Type-safe option handling
(define get-or-default [opt {(Option {Int})}] [default {Int}] {Int}
  (match opt
    [(Some value)] value
    [(None)] default))
```

---

## 7. Regular Expressions

OmniLisp provides a powerful regex engine based on Pika-style PEG matching. Unlike traditional backtracking regex engines, this implementation uses deterministic parsing with ordered choice.

### 7.1 Regex Functions

| Function | Description |
|----------|-------------|
| `(re-match pattern input)` | Find first match anywhere in string |
| `(re-fullmatch pattern input)` | Check if pattern matches entire string |
| `(re-find-all pattern input)` | Find all non-overlapping matches |
| `(re-split pattern input)` | Split string by pattern |
| `(re-replace pattern replacement input global?)` | Replace matches |

#### Examples

```lisp
;; Find first match
(re-match "[0-9]+" "abc123def")        ;; → "123"

;; Full string match
(re-fullmatch "[a-z]+" "hello")        ;; → true
(re-fullmatch "[a-z]+" "hello123")     ;; → false

;; Find all matches
(re-find-all "[0-9]+" "a1b23c456")     ;; → (1 23 456)

;; Split by pattern
(re-split "," "a,b,c,d")               ;; → (a b c d)

;; Replace (global=true for all, false for first only)
(re-replace "[0-9]" "X" "a1b2c3" true) ;; → "aXbXcX"
```

### 7.2 Pattern Syntax

#### Basic Patterns

| Pattern | Description | Example |
|---------|-------------|---------|
| `abc` | Literal characters | `"abc"` matches `"abc"` |
| `.` | Any single character | `"a.c"` matches `"abc"`, `"axc"` |
| `\|` | Alternation | `"cat\|dog"` matches `"cat"` or `"dog"` |
| `(...)` | Grouping | `"(ab)+"` matches `"abab"` |
| `(?:...)` | Non-capturing group | `"(?:ab)+"` same as above, no capture |

#### Character Classes

| Pattern | Description | Example |
|---------|-------------|---------|
| `[abc]` | Match any of a, b, c | `"[aeiou]"` matches vowels |
| `[a-z]` | Character range | `"[a-zA-Z]"` matches letters |
| `[^abc]` | Negated class | `"[^0-9]"` matches non-digits |

#### Escape Sequences

| Escape | Meaning | Equivalent |
|--------|---------|------------|
| `\d` | Digit | `[0-9]` |
| `\D` | Non-digit | `[^0-9]` |
| `\w` | Word character | `[a-zA-Z0-9_]` |
| `\W` | Non-word character | `[^a-zA-Z0-9_]` |
| `\s` | Whitespace | `[ \t\n\r]` |
| `\S` | Non-whitespace | `[^ \t\n\r]` |
| `\n` | Newline | - |
| `\t` | Tab | - |
| `\r` | Carriage return | - |
| `\\` | Literal backslash | - |

#### Anchors

| Anchor | Description | Example |
|--------|-------------|---------|
| `^` | Start of string | `"^hello"` matches at start |
| `$` | End of string | `"world$"` matches at end |
| `\b` | Word boundary | `"\bword\b"` matches whole word |

```lisp
(re-match "^hello" "hello world")      ;; → "hello"
(re-match "world$" "hello world")      ;; → "world"
(re-match "\\bcat\\b" "the cat sat")   ;; → "cat"
```

#### Quantifiers

| Quantifier | Description | Example |
|------------|-------------|---------|
| `*` | Zero or more | `"a*"` matches `""`, `"a"`, `"aaa"` |
| `+` | One or more | `"a+"` matches `"a"`, `"aaa"` |
| `?` | Zero or one | `"a?"` matches `""`, `"a"` |
| `{n}` | Exactly n | `"a{3}"` matches `"aaa"` |
| `{n,}` | n or more | `"a{2,}"` matches `"aa"`, `"aaa"`, ... |
| `{n,m}` | Between n and m | `"a{2,4}"` matches `"aa"` to `"aaaa"` |

```lisp
(re-match "a{3}" "aaaaa")              ;; → "aaa"
(re-match "a{2,4}" "aaaaa")            ;; → "aaaa" (greedy)
(re-match "a{2,}" "aaaaa")             ;; → "aaaaa"
```

#### Possessive Quantifiers

Possessive quantifiers never backtrack, providing explicit control over matching behavior. In PEG-based matching, all quantifiers are naturally possessive.

| Quantifier | Description |
|------------|-------------|
| `*+` | Possessive zero or more |
| `++` | Possessive one or more |
| `?+` | Possessive optional |
| `{n,m}+` | Possessive bounded |

```lisp
(re-match "a*+b" "aaab")               ;; → "aaab"
(re-match "a++b" "aaab")               ;; → "aaab"
```

### 7.3 Lookahead

Lookahead assertions check if a pattern matches (or doesn't match) at the current position without consuming input. This is a key PEG feature.

| Syntax | Description |
|--------|-------------|
| `(?=...)` | Positive lookahead - succeeds if pattern matches |
| `(?!...)` | Negative lookahead - succeeds if pattern does NOT match |

```lisp
;; Positive lookahead: match "foo" only if followed by "bar"
(re-match "foo(?=bar)" "foobar")       ;; → "foo"
(re-match "foo(?=bar)" "foobaz")       ;; → nothing

;; Negative lookahead: match "foo" only if NOT followed by "bar"
(re-match "foo(?!bar)" "foobaz")       ;; → "foo"
(re-match "foo(?!bar)" "foobar")       ;; → nothing

;; Match digits only if followed by non-digit
(re-match "\\d+(?=\\D)" "123abc")      ;; → "123"
```

### 7.4 PEG vs Traditional Regex

OmniLisp's regex engine is based on **Parsing Expression Grammars (PEG)**, which differ from traditional regex:

| Feature | PEG (OmniLisp) | Traditional Regex |
|---------|----------------|-------------------|
| Matching | Deterministic, greedy | Backtracking |
| Alternation | Ordered choice (`a\|b` tries `a` first) | Unordered |
| Quantifiers | Possessive (no backtrack) | Greedy + backtrack |
| Lookahead | Native support | Extension |
| Backreferences | Not supported | Supported |
| Performance | O(n) guaranteed | Worst-case exponential |

**Key implications:**
- No catastrophic backtracking
- Alternation order matters: `cat|catalog` matches `"cat"` in `"catalog"`
- Use negative lookahead for "not followed by" patterns
- Backreferences (`\1`, `\2`) are not available

### 7.5 Common Patterns

```lisp
;; Match email-like pattern
(re-match "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}"
          "contact@example.com")

;; Match integers (including negative)
(re-match "-?[0-9]+" "value is -42")   ;; → "-42"

;; Match quoted strings
(re-match "\"[^\"]*\"" "say \"hello\"") ;; → "\"hello\""

;; Match words at word boundaries
(re-match "\\bthe\\b" "other the end")  ;; → "the"

;; Match URL protocol
(re-match "https?://" "https://example.com") ;; → "https://"

;; Stop before delimiter (using lookahead)
(re-match "[^,]+(?=,)" "a,b,c")        ;; → "a"
```

---

## 8. Pika Grammar DSL

Define high-performance PEG grammars directly in the Flow domain.

```lisp
(define [grammar arithmetic]
  [expr (first (seq (ref term) "+" (ref expr)) (ref term))]
  [term (ref factor)]
  ...)
```

---

## 8. Memory Management (ASAP & RC-G)

OmniLisp utilizes a unique, garbage-collection-free memory model designed for predictability and performance.

### 8.1 ASAP (As Static As Possible)
The compiler statically analyzes variable lifetimes and automatically injects deallocation calls. This minimizes runtime overhead and eliminates "stop-the-world" pauses.

### 8.2 RC-G (Region-Based Reference Counting)
For dynamic data, OmniLisp uses **Region-Based Reference Counting**.
*   **Regions**: Objects are allocated into regions (arenas) that track their own internal connectivity.
*   **Transmigration**: The system can move objects between regions to optimize locality and clear cycles.
*   **Tethers**: Thread-local references that prevent premature deallocation while a value is in active use.

#### Phase 24 Performance Optimizations (2026-01-08)

The RC-G model has been heavily optimized with 9 major improvements achieving 2.7x-21.1x speedups:

| Optimization | Benefit |
|--------------|---------|
| Inline allocation buffer | 6.99x faster for small objects (< 64 bytes) |
| Specialized constructors | 5.55-6.32x faster batch list/tree allocation |
| Bitmap-based cycle detection | 2.7-12.5x faster transmigration |
| Region splicing | O(1) result-only region transfer (1.4-1.9x faster) |
| Region pooling | 21.1x faster small region creation |
| Inline fastpaths | Zero call overhead for hot operations |

**Key implementation details:**
- Regions use a 512-byte inline buffer for small object allocation
- Bitmap-based cycle detection replaces hash tables for O(1) visited tracking
- Thread-local region pool (32 regions per thread) eliminates malloc overhead
- Region splicing provides O(1) arena chunk transfer for functional patterns

See `runtime/bench/BENCHMARK_RESULTS.md` for detailed performance data.

---

## 9. Deprecated Namespaces

The following are legacy and should not be used:
*   `violet.*`: Use core primitives instead.
*   `scicomp.*`: Pending modern refactor.

---

## Appendix A: Type Definition Reference

The `(define ...)` form unifies all top-level definitions.

### Abstract Types
```lisp
(define {abstract Animal} [])
(define ^:parent {Animal} {abstract Mammal} [])
```

### Concrete Structs (Immutable Default)
```lisp
(define {struct Point}
  [x {Float}]
  [y {Float}])

;; With Parent Type (Metadata Syntax)
(define ^:parent {Shape} {struct Circle}
  [center {Point}]
  [radius {Float}])
```

### Parametric Types
```lisp
(define {struct [Pair T]}
  [first {T}]
  [second {T}])

;; Parametric with Parent
(define ^:parent {Any} {struct [Entry K V]}
  [key {K}]
  [value {V}])
```

### Mutable Structs
```lisp
(define {struct Player}
  [^:mutable hp {Int}]  ; Field-level mutability
  [name {String}])

;; Whole struct mutable sugar
(define ^:mutable {struct Player} ...)
```

### Enums (Sum Types)
```lisp
(define {enum Color} Red Green Blue)

(define {enum Option T}
  (Some [value {T}])
  None)
```

---

## Appendix B: Function Parameter Forms

| Syntax | Meaning |
|--------|---------|
| `x` | Untyped parameter |
| `[x {Int}]` | Typed parameter |
| `[x 10]` | Parameter with default value |
| `[x {Int} 10]` | Typed parameter with default |


## Tower of interpeters
```lisp
(lift val)           ; value → code
(run code)           ; execute code (JIT)
(EM expr)            ; meta-level evaluation
(shift n expr)       ; go up n levels
(meta-level)         ; current tower level
(clambda (x) body)   ; compile lambda

```

## Lexical Rules

### Symbol Character Rules

Symbols can contain letters and certain operators, but have specific rules about which characters can appear where.

**Start with (first character):**
- Letters: `a-z`, `A-Z`
- Operators: `*`, `!`, `-`, `_`, `?`, `%`, `/`, `=`, `<`, `>`

**Excluded from start:**
- Digits: `0-9` (to avoid confusion with integers)
- Reserved syntax: `.`, `@`, `#`, `:`, `;`
- Note: `&` is reserved for pattern guards in match expressions

**Middle/subsequent characters:**
- All of the above (letters + operators)
- **Plus:** Digits `0-9`

**Excluded entirely:**
- `.` - Used for module paths (`Math.sin`)
- `@` - Used for metadata (`^:where`)
- `#` - Used for reader macros (`#val`, `#\newline`)
- `&` - Used for pattern guards: `[x & (> x 10)]`
- `:` - Used for type annotations (`{Type}`)
- `;` - Used for comments

**Convention (not enforced):**
`!` and `?` are typically only at the **start or end** of symbols:
- At end: `set!`, `define!`, `null?`, `empty?`
- At start: `!not`, `!null`, `?maybe`, `?value`
- Not in middle: `foo!bar`, `set!value` (conventionally weird)

**Examples:**
```scheme
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
+                  ; single operator
<=                 ; comparison operators
==                 ; equality
50%off             ; % in middle
3/4                ; / in middle

; Invalid (can't start with digits)
123foo             ; integer, not symbol
3d                 ; digit first
7up                ; digit first

; Invalid (reserved for syntax)
.foo               ; . for paths
foo.bar            ; . for paths (not a single symbol)
@meta              ; @ for metadata
#reader           ; # for reader macros
&and               ; & excluded
:type              ; : for types
comment;more       ; ; for comments
```

## Data Types
```scheme
42              ; integer
3.14            ; float
'foo            ; symbol
#\a #\newline   ; character
'(1 2 3)        ; list
()              ; empty list (truthy)
nothing         ; nothing (falsy)
false           ; false (falsy)
true            ; true
0               ; integer (truthy)
```

## Special Forms
**Implemented subset (current C compiler/runtime):** `define`, `lambda`/`fn`, `let`, `let*`, `if`, `do`/`begin`, and `quote`. Other forms listed below are design target.
```scheme
; Let bindings with destructuring
(let [x 10] [y 20]                    ; Simple bindings
  (+ x y))

(let [[x y z] my-vec]                 ; Array destructuring
  (+ x y z))

(let [(Point x y) my-point]            ; Constructor destructuring
  (* x y))

(let [x {Int} 10] [y {Int} 20]        ; Typed bindings
  (+ x y))

(let ^:seq [x 1] [y (+ x 1)]         ; Sequential (like let*)
  (* x y))

; Traditional list-style also supported
(let ((x 10) (y 20))
  (+ x y))

; Lambda/function
(lambda (x) body)                    ; Function
(lambda self (x) body)               ; Recursive function
(fn x y (* x y))                     ; Shorthand (without parens)

; Conditionals and control flow
(if cond then else)                  ; Conditional (desugars to match)
(and e1 e2 ...)                      ; Short-circuit and
(or e1 e2 ...)                       ; Short-circuit or
(do e1 e2 ... en)                    ; Sequence, return last

; Pattern matching (match is the source of truth)
(match expr
  pattern1 result1
  pattern2 result2
  ...
  _ default)                         ; Wildcard fallback

;; Match examples:
;; Simple value matching
(match x
  1 "one"
  2 "two"
  _ "other")

;; Destructuring patterns
(match [1 2 3]
  [x y z] (+ x y z)                  ; Bind x=1, y=2, z=3
  _ 0)

;; Guards (& clauses)
(match x
  [n {Int} & (> n 10)] "large"       ; Type constraint + guard
  [n {Int}] "small"
  _ "not an int")

;; Nested destructuring
(match (list 1 (list 2 3))
  [1 [x y]] (+ x y)                  ; x=2, y=3
  _ 0)

;; Splicing patterns
(match my-list
  [x y .. rest] (cons x (cons y rest))  ; Bind first two, capture rest
  _ nothing)

; Quoting
(quote x) / 'x                       ; Quote
```
