# Pattern Matching Guide

Pattern matching is the foundation of OmniLisp's control flow. All conditionals (`if`, `when`, `cond`) desugar to `match` at parse time. This guide covers the full pattern matching system.

## Core Principle: Match is Truth

OmniLisp has no primitive `if` statement. Everything is pattern matching:

```lisp
;; This:
(if condition then-expr else-expr)

;; Is syntactic sugar for:
(match condition
  true  then-expr
  false else-expr)
```

This unification simplifies the mental model - there's only one way to branch.

## Basic Match Syntax

```lisp
(match scrutinee
  pattern1 result1
  pattern2 result2
  pattern3 result3)
```

Patterns and results alternate. Patterns are tested in order; the first match wins.

## Pattern Types

### Literal Patterns

Match exact values (bare literals):

```lisp
(match n
  0     "zero"
  1     "one"
  2     "two"
  _     "other")

(match s
  "yes"  true
  "no"   false
  _      nothing)
```

### Binding Patterns

Capture the value in a variable (bare symbol):

```lisp
(match value
  x x)  ;; Always matches, binds to x

(match (list 1 2 3)
  xs (length xs))  ;; -> 3
```

### Wildcard Pattern

Match anything, bind nothing (bare `_`):

```lisp
(match value
  _ "matches anything")

;; Useful as catch-all
(match result
  0     "zero"
  1     "one"
  _     "something else")
```

### Constructor Patterns

Destructure algebraic data types:

```lisp
;; Define a type
(define Point [x {Int}] [y {Int}])

;; Match and destructure
(match p
  (Point x y) (+ x y))

;; Nested constructors
(define Line [start {Point}] [end {Point}])

(match line
  (Line (Point x1 y1) (Point x2 y2))
    (sqrt (+ (square (- x2 x1))
             (square (- y2 y1)))))
```

### List Patterns

#### Empty List

```lisp
(match items
  ()  "empty"
  _   "not empty")
```

#### Exact List

```lisp
(match items
  (a)       "one element"
  (a b)     "two elements"
  (a b c)   "three elements"
  _         "other")
```

#### Head and Tail (Spread Pattern)

```lisp
;; Match first element and rest
(match items
  (h .. t)  (do
              (println (str "first: " h))
              (println (str "rest: " t))))

;; Multiple elements at head
(match items
  (a b .. rest)  (str a "-" b " and " (length rest) " more"))

;; Build recursive functions
(define sum
  ()        0
  (h .. t)  (+ h (sum t)))
```

### Array Patterns

Arrays use bracket syntax:

```lisp
(match arr
  []           "empty array"
  [a]          "single element"
  [a b]        "pair"
  [a b .. rest] (str "starts with " a " and " b))
```

### Dict Patterns

Extract values by key:

```lisp
(match person
  #{"name" n "age" a}
    (str n " is " a " years old"))

;; Partial matching (doesn't require all keys)
(match config
  #{"port" p}  (str "port: " p))
```

### Or Patterns

Match multiple alternatives:

```lisp
(match value
  (or 0 1)  "binary"
  _         "not binary")

(match day
  (or "Saturday" "Sunday")  "weekend"
  _                         "weekday")
```

### Guard Clauses

Add boolean conditions with `&`:

```lisp
(match n
  [x & (> x 100)]  "large"
  [x & (> x 10)]   "medium"
  [x & (> x 0)]    "small"
  0                "zero"
  _                "negative")

;; Multiple conditions
(match point
  (Point x y) & (= x y)          "on diagonal"
  (Point x y) & (> x 0) (> y 0)  "in quadrant 1"
  _                               "elsewhere")
```

### Predicate Patterns

Apply a predicate function:

```lisp
(match value
  [x & (even? x)]  "even"
  [x & (odd? x)]   "odd")

;; Using built-in predicates
(match data
  [x & (list? x)]   "it's a list"
  [x & (array? x)]  "it's an array"
  [x & (dict? x)]   "it's a dict"
  _                 "something else")
```

### View Patterns

Transform before matching:

```lisp
;; Match on transformed value
(match text
  (-> length n) & (> n 100)  "long text"
  (-> length n) & (> n 10)   "medium text"
  _                           "short text")

;; Extract and match
(match response
  (-> get-status 200)   "OK"
  (-> get-status 404)   "Not Found"
  (-> get-status code)  (str "Status: " code))
```

### As-Patterns (Capture whole + parts)

```lisp
(match list
  whole as (h .. t)
    (str "full list: " whole ", head: " h))
```

## Pattern Matching in Definitions

### Multi-Clause Functions

Define functions with pattern-matched arguments:

```lisp
;; Factorial
(define factorial
  0  1
  n  (* n (factorial (- n 1))))

;; Fibonacci
(define fib
  0  0
  1  1
  n  (+ (fib (- n 1)) (fib (- n 2))))

;; List length
(define length
  ()       0
  (_ .. t) (+ 1 (length t)))
```

### Destructuring in Let

```lisp
;; Destructure lists
(let [(a b c) '(1 2 3)]
  (+ a b c))  ;; -> 6

;; Destructure tuples/constructors
(let [(Point x y) my-point]
  (* x y))

;; Nested destructuring
(let [(a (b c) d) nested-list]
  (list a b c d))
```

### Destructuring in Lambda

```lisp
;; Lambda with destructuring
(map (lambda [(Point x y)] (+ x y)) points)

;; With spread
(map (lambda [(h .. t)] h) lists)
```

## Advanced Patterns

### Type-Constrained Patterns

```lisp
(match value
  [x {Int}]     (str "integer: " x)
  [x {String}]  (str "string: " x)
  [x {List}]    (str "list of " (length x)))
```

### Nested Patterns

Patterns can nest arbitrarily deep:

```lisp
(match tree
  (Node (Node ll lv lr) v (Node rl rv rr))
    "full binary node"
  (Node Empty v Empty)
    "leaf"
  (Node left v right)
    "partial node")
```

### Pattern Exhaustiveness

OmniLisp warns when patterns don't cover all cases:

```lisp
;; Warning: non-exhaustive patterns
(match bool-value
  true "yes")
;; Missing: false case

;; Fixed:
(match bool-value
  true  "yes"
  false "no")

;; Or with wildcard:
(match bool-value
  true "yes"
  _    "no")
```

## Common Patterns

### Option/Maybe Handling

```lisp
(define Maybe [T]
  Nothing
  (Just [value {T}]))

(match result
  Nothing   (println "no value")
  (Just x)  (println (str "got: " x)))

;; Chain with bind
(define maybe-map [f]
  Nothing  Nothing
  (Just x) (Just (f x)))
```

### Result/Either Handling

```lisp
(define Result [T E]
  (Ok [value {T}])
  (Err [error {E}]))

(match operation
  (Ok value)  (process value)
  (Err msg)   (println (str "Error: " msg)))
```

### List Processing

```lisp
;; Map
(define map [f]
  ()       '()
  (h .. t) (cons (f h) (map f t)))

;; Filter
(define filter [pred]
  ()       '()
  (h .. t) (if (pred h)
               (cons h (filter pred t))
               (filter pred t)))

;; Fold
(define foldl [f acc]
  ()       acc
  (h .. t) (foldl f (f acc h) t))
```

### Tree Traversal

```lisp
(define Tree [T]
  Empty
  (Node [left {(Tree T)}] [value {T}] [right {(Tree T)}]))

(define inorder
  Empty              '()
  (Node left v right)
    (append (inorder left)
            (cons v (inorder right))))

(define depth
  Empty              0
  (Node left _ right)
    (+ 1 (max (depth left) (depth right))))
```

### State Machines

```lisp
(define State
  Idle
  (Running [progress {Int}])
  (Paused [at {Int}])
  (Completed [result {Any}])
  (Failed [error {String}]))

(define handle-event [state event]
  ;; From Idle
  Idle :start
    (Running 0)

  ;; From Running
  (Running p) :pause
    (Paused p)
  (Running p) :progress
    (Running (+ p 1))
  (Running p) :complete
    (Completed (get-result p))
  (Running _) :error
    (Failed (get-error))

  ;; From Paused
  (Paused p) :resume
    (Running p)

  ;; Default: no state change
  (state _)
    state)
```

## Pattern Matching vs. Multiple Dispatch

Both features exist in OmniLisp but serve different purposes:

| Feature | Pattern Matching | Multiple Dispatch |
|---------|-----------------|-------------------|
| When | Runtime, per call | Compile-time type resolution |
| What | Data structure shape | Argument types |
| Where | Inside function body | Function signatures |

```lisp
;; Pattern matching - structural
(define process [data]
  (match data
    (User name age)  (str name " is " age)
    (Admin name)     (str name " (admin)")
    _                "unknown"))

;; Multiple dispatch - type-based
(define describe [x {Int}]    "integer")
(define describe [x {String}] "string")
(define describe [x {List}]   "list")
```

You can combine both:

```lisp
;; Dispatch on type, then match on structure
(define process [x {User}]
  (match x
    (User name age) & (> age 18)  "adult user"
    (User name _)                  "minor user"))

(define process [x {Admin}]
  (match x
    (Admin name) & (superuser? name)  "superuser"
    (Admin name)                       "regular admin"))
```

## Performance Considerations

1. **Pattern order matters** - Most specific patterns first
2. **Guards are checked after structural match** - Put cheap guards first
3. **Constructor patterns are O(1)** - Tag check is fast
4. **List spread is O(n)** - For rest of list
5. **Dict patterns hash-lookup** - O(1) per key

```lisp
;; Good: specific first, general last
(match n
  0              "zero"
  1              "one"
  x & (> x 100)  "large"
  x              "other")

;; Bad: general patterns shadow specific ones
(match n
  x              "other"     ;; Always matches!
  0              "zero"      ;; Never reached
  1              "one")      ;; Never reached
```

## Summary

| Pattern Type | Syntax | Example |
|-------------|--------|---------|
| Literal | `value` | `42`, `"hello"` |
| Binding | `name` | `x`, `result` |
| Wildcard | `_` | `_` |
| Constructor | `(Ctor args...)` | `(Point x y)` |
| Empty list | `()` | `()` |
| Spread | `(h .. t)` | `(first .. rest)` |
| Array | `[a b c]` | `[x y z]` |
| Or | `(or a b)` | `(or 0 1)` |
| Guard | `pat & cond` | `x & (> x 0)` |
| Dict | `#{key val}` | `#{"name" n}` |
| As | `name as pat` | `xs as (h .. t)` |
| View | `(-> fn pat)` | `(-> length 0)` |
| Typed | `[name {Type}]` | `[x {Int}]` |

## Further Reading

- [Quick Reference](QUICK_REFERENCE.md) - Language overview
- [Type System Guide](TYPE_SYSTEM_GUIDE.md) - Types and dispatch
- [API Reference](API_REFERENCE.md) - All functions
