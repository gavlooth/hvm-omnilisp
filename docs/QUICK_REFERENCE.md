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

### 2.4.2 Named Let (Scheme-Style Iteration)

Named let provides iteration via local recursive functions:

```lisp
;; (let name [bindings...] body)
;; 'name' becomes a recursive function within body

;; Sum integers 0 to 9
(let loop [i 0] [sum 0]
  (if (< i 10)
      (loop (+ i 1) (+ sum i))
      sum))  ; → 45

;; Factorial
(let fact [n 10] [acc 1]
  (if (<= n 1)
      acc
      (fact (- n 1) (* acc n))))  ; → 3628800
```

**Note:** OmniLisp does NOT have Clojure-style `loop`/`recur`. Use named let instead.

**Parallel vs Sequential Evaluation:**
```lisp
;; PARALLEL (default) - bindings evaluate simultaneously
(let [a (expensive 1)] [b (expensive 2)] (+ a b))

;; SEQUENTIAL (^:seq) - when bindings depend on each other
(let ^:seq [x 1] [y (+ x 1)] [z (* y 2)] z)  ; → 4
```

### 2.5 Type Enforcement
Every variable has an associated Kind. If no Kind is specified, it defaults to **`Any`**.
*   **Assignments:** `define`, `let`, and `set!` validate values against the Kind blueprint.
*   **Application:** `omni_apply` validates arguments against parameter Kinds and the result against the return Kind.

### 2.6 Gradual Multiple Dispatch

OmniLisp uses **Gradual Multiple Dispatch** - method selection based on all argument types, with compile-time resolution when types are known and runtime fallback for dynamic code.

**Dispatch is determined by:**
1.  **Arity:** The number of arguments provided.
2.  **Specificity:** The Kind of each argument (most specific match wins).
3.  **Timing:** Compile-time if types known, runtime otherwise.

```lisp
;; 1. Multi-arity (different argument counts)
(define area 0)                      ; 0 args
(define area [r] (* 3.14 (* r r)))   ; 1 arg

;; 2. Type Specialization (compile-time dispatch when typed)
(define describe [x {Int}] "An integer")
(define describe [x {String}] "A string")

;; 3. Untyped (runtime dispatch)
(define process [x] (fallback-handler x))

;; 4. Mixed (gradual - static where possible)
(define transform [x {Number}] [y] (+ x (coerce y)))
```

**Gradual spectrum:** Add type annotations incrementally to gain more compile-time guarantees.

### 2.7 Dispatch Resolution Algorithm

The dispatch algorithm follows this exact sequence:

#### Step 1: Arity Filtering
Only methods with matching arity (number of parameters) are considered.

```lisp
(defgfun area [shape])           ; 1-arity methods only
(defmethod area [c {Circle}] (* PI (* c.radius c.radius)))
(defmethod area [r {Rect}] (* r.width r.height))

(area my-circle)  ; Only 1-arity methods considered
```

#### Step 2: Type Matching
For each candidate method, check if argument types are subtypes of parameter types.

```lisp
;; Type hierarchy: Dog <: Animal <: Any
(defgfun speak [x])
(defmethod speak [x {Animal}] "generic sound")
(defmethod speak [x {Dog}] "woof")

(speak (Dog))  ; Both methods match! Dog <: Animal and Dog <: Dog
```

#### Step 3: Specificity Comparison
When multiple methods match, compare specificity **position by position**:

| Comparison Result | Meaning |
|-------------------|---------|
| `#ASpec{}` | Method A is strictly more specific |
| `#BSpec{}` | Method B is strictly more specific |
| `#Equal{}` | Same specificity (first defined wins) |
| `#Ambig{}` | Neither is more specific → **Error** |

**Specificity Rule:** A type is more specific if it's a proper subtype.

```lisp
;; Example: Dog <: Animal
;; Method 1: [x {Animal}] - specificity score: 0
;; Method 2: [x {Dog}]    - specificity score: 1 (Dog <: Animal)
;; → Method 2 wins
```

#### Step 4: Ambiguity Detection
Ambiguity occurs when two methods each win on different argument positions:

```lisp
(defgfun collide [a] [b])
(defmethod collide [a {Circle}] [b {Shape}] "circle-shape")
(defmethod collide [a {Shape}] [b {Circle}] "shape-circle")

(collide (Circle) (Circle))
;; Method 1: wins on arg 1 (Circle <: Shape)
;; Method 2: wins on arg 2 (Circle <: Shape)
;; → #Err{AmbiguousMethod} - neither is strictly more specific!
```

**Resolution:** Add a more specific method that resolves the ambiguity:
```lisp
(defmethod collide [a {Circle}] [b {Circle}] "circle-circle")
;; Now this method wins for (Circle, Circle) args
```

#### Step 5: Curried Partial Application
Multi-arity methods support partial application via `#GPrt`:

```lisp
(defgfun add [x] [y])
(defmethod add [x {Int}] [y {Int}] (+ x y))

(define add5 (add 5))  ; Returns #GPrt{add, methods, [5], 1}
(add5 10)              ; → 15 (completes dispatch)
```

#### Error Conditions

| Error | Cause |
|-------|-------|
| `#Err{NoMethod}` | No method matches argument types |
| `#Err{AmbiguousMethod}` | Multiple methods equally specific |
| `#Err{DispatchFailed}` | Internal dispatch error |

---

## 3. The Type System (Kind Domain)

OmniLisp uses **`define` for ALL definitions** including types. There is NO `deftype`, `defstruct`, `defenum`, or similar - everything uses the unified `define` form with placeholders.

### 3.1 Type Definitions via `define`

> **IMPORTANT:** OmniLisp does NOT have `deftype`, `defstruct`, `defenum`, or any `def*` special forms for types.
> All type definitions use `define` with slot placeholders `[field {Type}]`.

#### Struct-like Types (Product Types)
Define types by specifying fields in slot syntax:

```lisp
;; Define a Point type with two fields
(define Point [x {Int}] [y {Int}])

;; This creates:
;;   - Constructor: (Point 3 4)
;;   - Accessors: (Point-x p), (Point-y p)

;; With parent type
(define ^:parent {Shape} Circle [center {Point}] [radius {Float}])
```

#### Enum/Variant Types (Sum Types)
Define variants as bare symbols or constructors:

```lisp
;; Simple enum - bare symbols
(define Color
  Red
  Green
  Blue)

;; Usage: (Color.Red), (Color.Green), etc.

;; Variant with data (ADT)
(define Maybe [T]
  Nothing
  (Just [val {T}]))

;; Usage: Nothing, (Just 42)
```

#### Parametric Types
Type parameters go in the first slot:

```lisp
;; Parametric type with one parameter
(define List [T]
  Nil
  (Cons [head {T}] [tail {List T}]))

;; Parametric struct-like
(define Pair [T] [first {T}] [second {T}])
```

#### Union Types
Unions combine multiple types:

```lisp
;; Union type
(define IntOrString (union [{Int} {String}]))

;; Empty union is the Bottom type (no values possible)
(define Bottom (union []))
```

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
(define ^:parent {Number} Int)

;; Number is a subtype of Any (implicit, as Any is root)
(define Number)
```

*   **`Any`** is the universal supertype (root of the type hierarchy)
*   **`Union{}`** (empty union) is the universal subtype (bottom type, has no values)
*   **`Nothing`** is a concrete singleton type with the single value `nothing` (similar to Julia)

### 3.5 Types as Values (First-Class Kinds)

In OmniLisp, types themselves are values that can be passed around, stored, and referenced. This leads to three distinct patterns in definitions:

#### Pattern 1: Type Inheritance (Metadata)
Use `^:parent` to declare that a type is a subtype of another:
```lisp
;; Integer IS-A Number (inheritance via metadata)
(define ^:parent {Number} Integer)
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

### 4.1 Polymorphic Dispatch (Type-Preserving Operations)

All sequence functions use **polymorphic dispatch**: the input type determines the output type. This enables gradual laziness - if you start with a lazy iterator, operations return lazy iterators.

```lisp
;; Polymorphic dispatch - type flows through
(map f some-list)      ; → List (eager)
(map f some-array)     ; → Array (eager)
(map f some-dict)      ; → Dict (maps over values, preserves keys)
(map f some-iterator)  ; → Iterator (lazy)

;; Same pattern for all core functions
(filter pred coll)     ; preserves type
(take n coll)          ; preserves type
(drop n coll)          ; preserves type
(zip xs ys)            ; preserves type
(foldl f init coll)    ; works on all types
(length coll)          ; O(1) for arrays, O(n) for lists
(reverse coll)         ; preserves type
```

**Behavior by collection type:**

| Function | List | Array | Dict | Iterator |
|----------|------|-------|------|----------|
| `map` | → List | → Array | → Dict (values) | → Iterator |
| `filter` | → List | → Array | → Dict (by value) | → Iterator |
| `take`/`drop` | → List | → Array | — | → Iterator |
| `zip` | → List | → Array | — | → Iterator |
| `foldl`/`foldr` | immediate | immediate | immediate (values) | immediate |
| `length` | O(n) | O(1) | O(n) | consumes |
| `reverse` | → List | → Array | — | — |

### 4.2 Collection Realization

Convert between collection types using realization functions:

```lisp
(collect iter)         ; Iterator → List
(collect_array iter)   ; Iterator → Array
(collect_dict iter)    ; Iterator of pairs → Dict

;; Type-directed collection with `into`
(into [] seq)          ; → Array
(into #{} seq)         ; → Dict
(into '() seq)         ; → List
```

### 4.3 Lazy Iterators

Iterators are **pull-based lazy sequences**. Operations on iterators return wrapper nodes that defer computation until consumed.

*   `(range n)`: Returns an iterator from 0 to n-1.
*   `(range start end)`: Returns an iterator from start to end-1.
*   `(range start end step)`: With custom step.

**Iterator-specific operations:**
*   `(iter-next it)`: Pull next value, returns `(Just val)` or `nothing`.
*   `(iter-peek it)`: Look at next without consuming.
*   `(enumerate it)`: Pairs each value with its index.
*   `(zip it1 it2 ...)`: Combine multiple iterators.
*   `(chain it1 it2 ...)`: Concatenate iterators.
*   `(cycle it)`: Infinite repetition.

### 4.4 Realization (Consuming Iterators)

Type constructors double as converters via gradual dispatch:
*   `(list iter)`: Collect iterator to list. Also `(list 1 2 3)` creates list.
*   `(array iter)`: Collect iterator to array. Also `(array 1 2 3)` creates array.
*   `(set iter)`: Collect iterator to set. Also `(set 1 2 3)` creates set.
*   `(dict iter)`: From iterator of pairs. Also `(dict :a 1 :b 2)` creates dict.

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

### 6.0 Pattern Matching (Primary Control Flow)

**`match` is the single source of truth for all conditional logic.** All other conditionals (`if`, `when`, `unless`) are syntactic sugar that desugar to `match` at parse time.

```lisp
;; Basic match (flat pattern-result pairs)
(match value
  1             "one"
  2             "two"
  x & (> x 10)  "large"
  _             "other")

;; Destructuring patterns
(match point
  (Point x y)  (+ x y)
  _            0)

;; Rest patterns
(match items
  [first second .. rest]  (list first (length rest))
  _                       nothing)
```

**Conditionals are syntactic sugar:**
| Form | Desugars To |
|------|-------------|
| `(if c t e)` | `(match c true t _ e)` |
| `(when c b)` | `(match c true b _ nothing)` |
| `(unless c b)` | `(match c true nothing _ b)` |

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
*   **`fiber`**: Creates a paused fiber from a thunk.
*   **`resume`**: Manually steps into a fiber (direct control).
*   **`yield`**: Suspends the current fiber, returning control to the caller or scheduler.
*   **`with-fibers`**: Establishes a local **Fiber Scheduler** scope. The block waits until all spawned fibers complete.
*   **`spawn`**: Registers a fiber with the current scheduler.
*   **`join`**: Blocks the current fiber until the target fiber completes, returning its result.
*   **`run-fibers`**: Explicitly runs the scheduler loop until all pending fibers are done.

#### Communication via Effects
Fibers communicate using **algebraic effects** rather than Go-style channels. Since HVM4 has no shared mutable state, effects provide a natural way for fibers to interact with the scheduler and each other without requiring channel synchronization primitives.

---

## 7. Pika Grammar DSL

Define high-performance PEG grammars directly in the Flow domain.

```lisp
(define [grammar arithmetic]
  [expr (first (seq (ref term) "+" (ref expr)) (ref term))]
  [term (ref factor)]
  ...)
```

---

## 8. Regular Expressions (Pika Regex)

OmniLisp includes a **Pika-style regex engine** - a self-contained PEG-based matcher that compiles regex patterns to an AST and matches directly.

### 8.1 Regex Functions

| Function | Args | Returns | Description |
|----------|------|---------|-------------|
| `re-match` | `pattern input` | `string \| nil` | First match anywhere in string |
| `re-find-all` | `pattern input` | `(list ...)` | All non-overlapping matches |
| `re-split` | `pattern input` | `(list ...)` | Split string by pattern |
| `re-replace` | `pattern repl input global?` | `string` | Replace matches (global if true) |
| `re-fullmatch` | `pattern input` | `bool` | True if entire string matches |

### 8.2 Pattern Syntax

```lisp
;; Literals
(re-match "hello" "say hello world")  ; => "hello"

;; Character classes
(re-match "[aeiou]" "hello")          ; => "e"
(re-match "[a-z]+" "Hello123")        ; => "ello"
(re-match "[^0-9]+" "abc123")         ; => "abc"

;; Quantifiers
(re-match "a*" "aaab")                ; => "aaa"
(re-match "a+" "aaab")                ; => "aaa"
(re-match "a?" "abc")                 ; => "a"
(re-match "a{2,4}" "aaaaa")           ; => "aaaa"

;; Possessive quantifiers (no backtracking)
(re-match "a*+" "aaab")               ; => "aaa" (greedy, no backtrack)

;; Alternation
(re-match "cat|dog" "I have a dog")   ; => "dog"

;; Grouping
(re-match "(ab)+" "ababab")           ; => "ababab"
(re-match "(?:ab)+" "ababab")         ; => "ababab" (non-capturing)

;; Lookahead
(re-match "foo(?=bar)" "foobar")      ; => "foo" (positive lookahead)
(re-match "foo(?!baz)" "foobar")      ; => "foo" (negative lookahead)

;; Anchors
(re-fullmatch "^hello$" "hello")      ; => true
(re-match "\\bword\\b" "a word here") ; => "word" (word boundary)

;; Escape sequences
(re-find-all "\\d+" "a1b23c456")      ; => ("1" "23" "456")
(re-match "\\w+" "hello_world!")      ; => "hello_world"
(re-match "\\s+" "a   b")             ; => "   "
```

### 8.3 Escape Reference

| Escape | Meaning |
|--------|---------|
| `\d` | Digit `[0-9]` |
| `\D` | Non-digit `[^0-9]` |
| `\w` | Word char `[a-zA-Z0-9_]` |
| `\W` | Non-word char |
| `\s` | Whitespace `[ \t\n\r]` |
| `\S` | Non-whitespace |
| `\n` | Newline |
| `\t` | Tab |
| `\r` | Carriage return |
| `\\` | Literal backslash |

### 8.4 Examples

```lisp
;; Split by whitespace
(re-split "\\s+" "hello   world  test")
; => ("hello" "world" "test")

;; Replace all digits
(re-replace "\\d" "X" "a1b2c3" true)
; => "aXbXcX"

;; Validate email (simplified)
(re-fullmatch "[a-z]+@[a-z]+\\.[a-z]+" "user@example.com")
; => true

;; Extract numbers
(re-find-all "-?\\d+\\.?\\d*" "temp: -3.5, count: 42")
; => ("-3.5" "42")
```

---

## 9. Memory Management (Region-RC)

OmniLisp uses **Region-RC**: scope-based regions with reference counting at
region granularity. No stop-the-world garbage collector.

- **Regions:** Objects allocate in regions with O(1) bump-pointer allocation.
- **Region-level RC:** Cross-region refs increment the target region's external_rc.
  A region is freed when: scope ends AND external_rc == 0.
- **Transmigration (escapes):** Values that cross a region boundary are copied
  into an outliving region to maintain region closure.
- **Tethering (borrows):** Borrow windows pin regions so they cannot be reclaimed
  while in use.

Canonical references:

- `docs/REGION_RC.md` (normative spec)
- `runtime/docs/CTRR_TRANSMIGRATION.md` (detailed transmigration contract)

---

## 10. Deprecated Namespaces

The following are legacy and should not be used:
*   `violet.*`: Use core primitives instead.
*   `scicomp.*`: Pending modern refactor.

---

## Appendix A: Type Definition Reference

OmniLisp uses `define` for ALL type definitions. There is NO `deftype`, `defstruct`, `defenum`, or any `def*` forms.

### Struct-like Types (Product Types)
```lisp
;; Basic struct - fields in slot syntax
(define Point [x {Float}] [y {Float}])

;; With parent type via metadata
(define ^:parent {Shape} Circle [center {Point}] [radius {Float}])

;; Mutable fields
(define Player [^:mutable hp {Int}] [name {String}])

;; Whole type mutable
(define ^:mutable GameState [score {Int}] [level {Int}])
```

### Parametric Types
```lisp
;; Type parameter in first slot
(define Pair [T] [first {T}] [second {T}])

;; Multiple type parameters
(define Entry [K V] [key {K}] [value {V}])

;; With parent
(define ^:parent {Any} Entry [K V] [key {K}] [value {V}])
```

### Enum/Sum Types (ADTs)
```lisp
;; Simple enum - bare variant names
(define Color Red Green Blue)

;; ADT with data variants
(define Option [T]
  None
  (Some [value {T}]))

;; More complex ADT
(define Expr
  (Lit [value {Int}])
  (Add [left {Expr}] [right {Expr}])
  (Mul [left {Expr}] [right {Expr}]))
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
- Reserved syntax: `.`, `@`, `#`, `&`, `:`, `;`

**Middle/subsequent characters:**
- All of the above (letters + operators)
- **Plus:** Digits `0-9`

**Excluded entirely:**
- `.` - Used for module paths (`Math.sin`)
- `@` - Used for metadata (`^:where`)
- `#` - Used for reader macros (`#val`, `#\newline`)
- `&` - Excluded
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

; Lambda/function
(lambda (x) body)                    ; Function
(lambda self (x) body)               ; Recursive function
(fn x y (* x y))                     ; Shorthand (without parens)

; Pattern matching (PRIMARY control flow)
(match value                         ; Pattern match expression
  pattern1        result1
  pattern2 & guard  result2
  _               default)

; Conditionals (syntactic sugar for match)
(if cond then else)                  ; → (match cond true then _ else)
(when cond body...)                  ; → (match cond true body _ nothing)
(unless cond body...)                ; → (match cond true nothing _ body)

; Boolean & sequencing
(and e1 e2 ...)                      ; Short-circuit and
(or e1 e2 ...)                       ; Short-circuit or
(do e1 e2 ... en)                    ; Sequence, return last

; Quoting
(quote x) / 'x                       ; Quote
(quasiquote x) / `x                  ; Quasiquote
(unquote x) / ,x                     ; Unquote in quasiquote
(unquote-splicing x) / ,@x           ; Splice in quasiquote
```
