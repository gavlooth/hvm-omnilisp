# Getting Started with OmniLisp

This guide will walk you through installing OmniLisp, understanding its core concepts, and writing your first programs.

## Prerequisites

- **HVM4** - The Higher-order Virtual Machine (runtime)
- **C compiler** (gcc or clang)
- **Make** (build tool)

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/your-org/hvm-omnilisp.git
cd hvm-omnilisp
```

### 2. Build OmniLisp

```bash
make
```

This compiles the C-based parser/compiler and prepares the HVM4 runtime.

### 3. Verify Installation

```bash
./omnilisp -e "(+ 1 2)"
# Output: 3
```

## Running OmniLisp

### Interactive REPL

```bash
./omnilisp
```

The REPL provides an interactive environment for exploring the language:

```
OmniLisp v0.1.0
>>> (+ 1 2)
3
>>> (define greet [name] (str "Hello, " name "!"))
#<function greet>
>>> (greet "World")
"Hello, World!"
>>> (exit)
```

### Running Files

Create a file `hello.ol`:

```lisp
;; hello.ol
(println "Hello, OmniLisp!")
```

Run it:

```bash
./omnilisp run hello.ol
# Output: Hello, OmniLisp!
```

### Evaluating Expressions

```bash
./omnilisp -e "(map (lambda [x] (* x x)) '(1 2 3 4 5))"
# Output: (1 4 9 16 25)
```

## The Character Calculus

OmniLisp uses a consistent bracket semantics called the **Character Calculus**. Each bracket type has a fixed meaning:

| Bracket | Domain | Purpose | Example |
|---------|--------|---------|---------|
| `()` | Flow | Execution, function calls | `(+ 1 2)` |
| `[]` | Slot | Parameters, arrays, patterns | `[x {Int}]` |
| `{}` | Kind | Type annotations | `{Int}`, `{(List Int)}` |

This consistency means you always know what a bracket means regardless of context.

## Your First Program

### Hello World

```lisp
(println "Hello, World!")
```

### Basic Arithmetic

```lisp
;; Arithmetic operations
(+ 1 2)       ;; -> 3
(- 10 4)      ;; -> 6
(* 3 7)       ;; -> 21
(/ 15 3)      ;; -> 5

;; Nested expressions
(+ (* 2 3) (* 4 5))  ;; -> 26
```

### Defining Values

```lisp
;; Define a constant
(define pi 3.14159)

;; Define with type annotation
(define answer {Int} 42)
```

### Defining Functions

OmniLisp uses **curried parameters** in slot syntax:

```lisp
;; Simple function (untyped)
(define add [x] [y]
  (+ x y))

;; Call it
(add 3 5)  ;; -> 8

;; Partially apply it
(define add5 (add 5))
(add5 10)  ;; -> 15
```

### Functions with Types

```lisp
;; Typed function
(define multiply [x {Int}] [y {Int}] {Int}
  (* x y))

;; Type annotations are optional - add them gradually
(define greet [name {String}] {String}
  (str "Hello, " name "!"))
```

## Working with Collections

### Lists

Lists are linked data structures, optimal for head operations:

```lisp
;; Create a list (quoted)
(define nums '(1 2 3 4 5))

;; List operations
(head nums)   ;; -> 1
(tail nums)   ;; -> '(2 3 4 5)
(cons 0 nums) ;; -> '(0 1 2 3 4 5)
(length nums) ;; -> 5
```

### Arrays

Arrays provide O(1) random access:

```lisp
;; Create an array (bracket syntax)
(define arr [10 20 30 40 50])

;; Array operations
(get arr 0)      ;; -> 10
(get arr 2)      ;; -> 30
(length arr)     ;; -> 5

;; Functional update
(set arr 1 25)   ;; -> [10 25 30 40 50]
```

### Dictionaries

Key-value mappings:

```lisp
;; Create a dict
(define person #{"name" "Alice" "age" 30})

;; Access values
(get person "name")  ;; -> "Alice"
(get person "age")   ;; -> 30

;; Add/update entries
(assoc person "city" "NYC")
;; -> #{"name" "Alice" "age" 30 "city" "NYC"}
```

## Higher-Order Functions

OmniLisp shines with functional programming:

```lisp
;; Map - transform each element
(map (lambda [x] (* x x)) '(1 2 3 4 5))
;; -> '(1 4 9 16 25)

;; Filter - keep matching elements
(filter even? '(1 2 3 4 5 6 7 8))
;; -> '(2 4 6 8)

;; Fold - reduce to single value
(foldl + 0 '(1 2 3 4 5))
;; -> 15

;; Compose operations
(-> '(1 2 3 4 5 6 7 8 9 10)
    (filter even?)
    (map (lambda [x] (* x x)))
    (foldl + 0))
;; -> 220
```

## Pattern Matching

Pattern matching is the foundation of OmniLisp's control flow:

```lisp
;; Basic pattern match
(match value
  0     "zero"
  1     "one"
  _     "other")

;; Binding patterns
(match point
  (Point x y)  (+ x y))

;; Guards
(match n
  x & (> x 0)  "positive"
  x & (< x 0)  "negative"
  _            "zero")

;; List patterns
(match items
  ()        "empty"
  (h .. t)  (str "first: " h))
```

### Functions via Pattern Matching

```lisp
;; Factorial using pattern-based definition
(define factorial
  0  1
  n  (* n (factorial (- n 1))))

(factorial 5)  ;; -> 120

;; Fibonacci
(define fib
  0  0
  1  1
  n  (+ (fib (- n 1)) (fib (- n 2))))

(fib 10)  ;; -> 55
```

## Algebraic Effects

OmniLisp uses **algebraic effects** instead of try/catch:

```lisp
;; Basic effect handling
(handle
  (if (< x 0)
      (perform fail "negative number")
      (sqrt x))
  (fail [msg resume]
    (println (str "Error: " msg))
    0))

;; The 'resume' parameter lets you continue execution
(handle
  (+ 1 (perform ask nothing))
  (ask [_ resume]
    (resume 42)))
;; -> 43
```

Effects provide more control than exceptions:

```lisp
;; Multiple effects
(handle
  (let [data (perform read-file "config.json")]
    (perform parse-json data))

  (read-file [path resume]
    (resume (slurp path)))

  (parse-json [str resume]
    (resume (json-parse str)))

  (error [msg _]
    (println (str "Error: " msg))
    #{}))
```

## Local Bindings

### Let Expressions

```lisp
;; Basic let
(let [x 10] [y 20]
  (+ x y))
;; -> 30

;; Typed let
(let [x {Int} 10] [y {Int} 20]
  (+ x y))

;; Destructuring
(let [[a b c] '(1 2 3)]
  (+ a b c))
;; -> 6
```

### Named Let (Iteration)

OmniLisp uses **named let** instead of loop/recur:

```lisp
;; Sum from 0 to 99
(let loop [[i 0] [sum 0]]
  (if (>= i 100)
      sum
      (loop (+ i 1) (+ sum i))))
;; -> 4950

;; Find first even number
(let search [[nums '(1 3 5 6 7 8)]]
  (match nums
    ()        nothing
    (h .. t)  (if (even? h)
                  h
                  (search t))))
;; -> 6
```

## Multiple Dispatch

Define multiple implementations based on argument types:

```lisp
;; Different implementations for different types
(define describe [x {Int}]    (str "integer: " x))
(define describe [x {String}] (str "string: " x))
(define describe [x {List}]   (str "list of " (length x) " items"))

(describe 42)         ;; -> "integer: 42"
(describe "hello")    ;; -> "string: hello"
(describe '(1 2 3))   ;; -> "list of 3 items"
```

## Example Programs

### FizzBuzz

```lisp
(define fizzbuzz [n]
  (match n
    _ & (= (mod n 15) 0)  "FizzBuzz"
    _ & (= (mod n 3) 0)   "Fizz"
    _ & (= (mod n 5) 0)   "Buzz"
    _                      (int->str n)))

(map fizzbuzz (range 1 16))
;; -> '("1" "2" "Fizz" "4" "Buzz" "Fizz" "7" "8" "Fizz" "Buzz"
;;      "11" "Fizz" "13" "14" "FizzBuzz")
```

### Quicksort

```lisp
(define quicksort
  ()  '()
  (pivot .. rest)
    (let [smaller (filter (lambda [x] (< x pivot)) rest)]
      (let [larger (filter (lambda [x] (>= x pivot)) rest)]
        (append (quicksort smaller)
                (cons pivot (quicksort larger))))))

(quicksort '(3 1 4 1 5 9 2 6))
;; -> '(1 1 2 3 4 5 6 9)
```

### Tree Traversal

```lisp
;; Define a binary tree type
(define Tree [T]
  Empty
  (Node [value {T}] [left {(Tree T)}] [right {(Tree T)}]))

;; In-order traversal
(define inorder
  Empty              '()
  (Node v left right)
    (append (inorder left)
            (cons v (inorder right))))

;; Create a tree
(define my-tree
  (Node 4
    (Node 2
      (Node 1 Empty Empty)
      (Node 3 Empty Empty))
    (Node 6
      (Node 5 Empty Empty)
      (Node 7 Empty Empty))))

(inorder my-tree)
;; -> '(1 2 3 4 5 6 7)
```

### Web Request with Effects

```lisp
(define fetch-user [id]
  (handle
    (let [response (perform http-get (str "/api/users/" id))]
      (let [data (perform parse-json response)]
        (get data "name")))

    (http-get [url resume]
      (resume (http-request "GET" url)))

    (parse-json [str resume]
      (resume (json-parse str)))

    (error [msg _]
      (str "Failed to fetch user: " msg))))
```

## Next Steps

Now that you have the basics:

1. **[Pattern Matching Guide](PATTERN_MATCHING_GUIDE.md)** - Deep dive into patterns
2. **[Type System Guide](TYPE_SYSTEM_GUIDE.md)** - Gradual typing and dispatch
3. **[Effects and Handlers](EFFECTS_AND_HANDLERS.md)** - Algebraic effects tutorial
4. **[Standard Library](STANDARD_LIBRARY.md)** - All prelude functions
5. **[API Reference](API_REFERENCE.md)** - Complete function reference
6. **[Quick Reference](QUICK_REFERENCE.md)** - Language overview card

## Tips for Success

1. **Think in patterns** - Use `match` instead of nested `if/else`
2. **Use effects for errors** - Replace try/catch with handle/perform
3. **Add types gradually** - Start untyped, add types as you refine
4. **Embrace currying** - Partial application is natural and powerful
5. **Check the Character Calculus** - When confused about brackets, remember: `()` executes, `[]` holds slots, `{}` describes types
