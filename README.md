# OmniLisp

**A modern Lisp dialect built on HVM4 with algebraic effects, multiple dispatch, and the Character Calculus**

OmniLisp combines the expressiveness of Lisp with Julia-style multiple dispatch, algebraic effects for error handling, and a unique bracket semantics called the "Character Calculus" that gives each delimiter a distinct semantic role.

## Features

- **Character Calculus** - Consistent bracket semantics: `()` for execution, `[]` for data/slots, `{}` for types
- **Pattern Matching** - All control flow desugars to `match` with guards, spread patterns, and more
- **Multiple Dispatch** - Julia-style dispatch on all argument types
- **Algebraic Effects** - `handle`/`perform`/`resume` instead of try/catch
- **Type System** - Gradual typing with inference, unions, intersections, and value types
- **Macro System** - Hygienic macros with ellipsis patterns
- **Tower of Interpreters** - Reflective meta-programming and staged computation
- **HVM4 Backend** - Automatic parallelism via interaction net reduction

## Quick Start

```lisp
;; Hello World
(println "Hello, OmniLisp!")

;; Function definition with types
(define add [x {Int}] [y {Int}] {Int}
  (+ x y))

;; Pattern matching
(define factorial
  [0]  1
  [n]  (* n (factorial (- n 1))))

;; Lists and higher-order functions
(define nums '(1 2 3 4 5))
(map (lambda [x] (* x x)) nums)  ;; -> '(1 4 9 16 25)

;; Algebraic effects instead of exceptions
(handle
  (if (< x 0)
      (perform fail "negative number")
      (sqrt x))
  (fail [msg resume]
    (println msg)
    0))

;; Multiple dispatch
(define describe [x {Int}]    (str "integer: " x))
(define describe [x {String}] (str "string: " x))
(define describe [x {List}]   (str "list of " (length x) " items"))
```

## Installation

```bash
# Clone the repository
git clone https://github.com/your-org/hvm-omnilisp.git
cd hvm-omnilisp

# Build with HVM4
make

# Run the REPL
./omnilisp

# Run a file
./omnilisp run examples/hello.ol

# Run an expression
./omnilisp -e "(+ 1 2)"
```

## The Character Calculus

OmniLisp uses a fixed semantic meaning for each bracket type:

| Character | Domain | Purpose | Example |
|-----------|--------|---------|---------|
| `()` | Flow | Execution, function calls | `(+ 1 2)` |
| `[]` | Slot | Parameters, arrays, patterns | `[x {Int}]`, `[1 2 3]` |
| `{}` | Kind | Type annotations | `{Int}`, `{(List T)}` |
| `^` | Meta | Metadata annotations | `^:mutable`, `^:ffi` |
| `.` | Path | Navigation, access | `obj.field`, `arr.[0]` |
| `#` | Reader | Dispatch, literals | `#{}` dict, `#\newline` |

## Core Concepts

### Match is Truth

All control flow desugars to pattern matching. There is no primitive `if`, `cond`, or `case`:

```lisp
;; if -> match
(if cond then else)
;; becomes:
(match cond
  true  then
  false else)

;; cond -> match with guards
(cond
  [(> x 0) "positive"]
  [(< x 0) "negative"]
  [true    "zero"])
;; becomes:
(match true
  _ & (> x 0) "positive"
  _ & (< x 0) "negative"
  _           "zero")
```

### Effects Over Exceptions

OmniLisp uses algebraic effects instead of try/catch:

```lisp
;; Define and handle effects
(handle
  (let [data (perform read-file "config.json")]
    (perform parse-json data))

  (read-file [path resume]
    (resume (slurp path)))

  (parse-json [str resume]
    (resume (json-parse str)))

  (error [msg _]
    (println "Error:" msg)
    #Noth{}))
```

### Named Let for Recursion

No `loop/recur`. Use named let for tail-recursive loops:

```lisp
(let loop [[i 0] [sum 0]]
  (if (>= i 100)
      sum
      (loop (+ i 1) (+ sum i))))
```

### Three Collection Types

```lisp
;; Lists (linked, optimal for head operations)
'(1 2 3 4 5)

;; Arrays (indexed, optimal for random access)
[1 2 3 4 5]

;; Dicts (key-value mapping)
#{"name" "Alice" "age" 30}
```

## Pattern Matching

OmniLisp has comprehensive pattern matching:

```lisp
;; Basic patterns
(match value
  0           "zero"           ;; Literal
  x           x                ;; Binding
  _           "anything"       ;; Wildcard
  (Point x y) (+ x y))         ;; Constructor

;; Advanced patterns
(match list
  ()          "empty"          ;; Empty list
  (h .. t)    (cons h t)       ;; Spread (head + rest)
  [a b c]     (+ a b c)        ;; Exact array
  (or x y)    "or pattern")    ;; Or pattern

;; Guards
(match n
  x & (> x 0)  "positive"
  x & (< x 0)  "negative"
  _            "zero")

;; Dict patterns
(match person
  #{"name" n "age" a} (str n " is " a))
```

## Type System

```lisp
;; Type annotations
(define greet [name {String}] {String}
  (str "Hello, " name "!"))

;; Union types
(define parse-int [s {String}] {union [{Int} {Nothing}]}
  (try-parse s))

;; Type aliases
(deftype-alias UserId {Int})
(deftype-alias Result {union [{Ok} {Err}]})

;; Intersection types (must satisfy all)
(define process [x {& [{Readable} {Sized}]}]
  (read-n x (size x)))

;; Bounded type variables
(define ^:where [T {Number}]
  add-nums [x {T}] [y {T}] {T}
  (+ x y))

;; Type inference
(typed-lambda [x] [y] (+ x y))  ;; Infers {Int} -> {Int} -> {Int}
```

## Standard Library

### Collections

```lisp
;; Polymorphic operations (work on List, Array, Dict, Iterator)
(map inc '(1 2 3))           ;; -> '(2 3 4)
(filter even? [1 2 3 4 5])   ;; -> [2 4]
(foldl + 0 '(1 2 3 4 5))     ;; -> 15
(take 3 '(1 2 3 4 5))        ;; -> '(1 2 3)
(drop 2 '(1 2 3 4 5))        ;; -> '(3 4 5)
(reverse [1 2 3])            ;; -> [3 2 1]
(sort '(3 1 4 1 5))          ;; -> '(1 1 3 4 5)

;; Searching
(find even? '(1 3 4 5))      ;; -> #Som{4}
(find-index even? '(1 3 4))  ;; -> #Som{2}

;; Grouping
(group-by even? '(1 2 3 4))  ;; -> #{true '(2 4) false '(1 3)}
(partition even? '(1 2 3 4)) ;; -> ['(2 4) '(1 3)]
```

### Lazy Iterators

```lisp
;; Lazy sequences
(lazy-range 0 1000000)       ;; No memory until realized
(iterate inc 0)              ;; Infinite sequence
(cycle '(1 2 3))             ;; -> 1 2 3 1 2 3 ...

;; Lazy operations
(-> (lazy-range 0 1000000)
    (lazy-filter even?)
    (lazy-map square)
    (lazy-take 10)
    (collect-list))          ;; Only computes 10 values
```

### I/O

```lisp
;; Console
(print "no newline")
(println "with newline")
(read_line _)

;; Files
(read_file "path.txt")
(write_file "path.txt" "content")
(read_lines "path.txt")

;; Directories
(list-dir "/path")
(find-files "/src" (lambda [f] (ends-with? f ".lisp")))
```

### Math

```lisp
;; Trigonometry
(sin x) (cos x) (tan x)
(asin x) (acos x) (atan x)

;; Exponentials
(exp x) (log x) (pow x n) (sqrt x)

;; Integer math
(abs x) (mod x y) (gcd a b) (lcm a b)

;; Floats
(float 314159 5)             ;; -> 3.14159
(float+ a b) (float* a b)
```

### Networking

```lisp
;; HTTP request
(http-get "example.com" 80 "/api/data")

;; TCP client
(let [sock (tcp-connect "localhost" 8080)]
  (tcp-send sock "Hello")
  (tcp-recv sock 1024))

;; TCP server
(tcp-serve 8080
  (lambda [client]
    (tcp-send client "Welcome!")))
```

### JSON

```lisp
;; Parse JSON
(json-parse "{\"name\": \"Alice\", \"age\": 30}")
;; -> #{"name" "Alice" "age" 30}

;; Stringify
(json-stringify #{"items" '(1 2 3) "active" true})
;; -> "{\"items\":[1,2,3],\"active\":true}"
```

### DateTime

```lisp
;; Current time
(datetime-now)

;; Formatting
(datetime-format (datetime-now) "%Y-%m-%d %H:%M:%S")

;; Arithmetic
(datetime-add-days (datetime-now) 7)
(datetime-diff dt1 dt2)
```

### Regex (Pika-based, no backtracking)

```lisp
;; Matching
(re-match "[0-9]+" "123")      ;; Full match
(re-test "[0-9]+" "abc123")    ;; Partial match

;; Search
(re-find "[0-9]+" "abc123def") ;; -> #Som{#CON{3, "123"}}
(re-find-all "[0-9]+" "a1b2")  ;; -> '((1 "1") (3 "2"))

;; Transform
(re-replace "[0-9]" "X" "a1b2") ;; -> "aXbX"
(re-split "[,;]" "a,b;c")       ;; -> '("a" "b" "c")
```

### Pika Grammar DSL

```lisp
;; Define grammar
(let [number (rule 'number (g-plus @g-digit))]
  (let [expr (rule 'expr
               (g-seq (g-ref 'number)
                      (g-star (g-seq (g-alt (g-lit "+") (g-lit "-"))
                                     (g-ref 'number)))))]
    (pika-parse (grammar '(number expr) 'expr) 'expr "1+2+3")))
```

## Macros

```lisp
;; Define a macro with pattern matching
(define [syntax when]
  [(when test body ...)
   (match test
     false nothing
     _     (do body ...))])

;; Use ellipsis for repetition
(define [syntax let*]
  [(let* () body)
   body]
  [(let* ([var val] rest ...) body)
   (let [var val] (let* (rest ...) body))])

;; Gensyms for hygiene
(define [syntax with-temp]
  [(with-temp body)
   (let [temp (gensym)] body)])
```

## Tower of Interpreters

OmniLisp supports multi-stage programming:

```lisp
;; Create code values
(code (+ 1 2))               ;; -> #Cod{#Add{#Lit{1}, #Lit{2}}}

;; Execute code
(code-exec (code (+ 1 2)))   ;; -> 3

;; Reflect values to code
(reflect 42)                 ;; -> #Cod{#Lit{42}}

;; Stage computation
(stage 1 (+ x y))            ;; Generate code at level 1

;; Partial evaluation
(pe-optimize (code (+ 1 2))) ;; -> #Cod{#Lit{3}}
```

## Fibers and Concurrency

```lisp
;; Create a fiber
(fiber-spawn (lambda [] (compute-something)))

;; Yield control
(yield value)

;; Run fibers cooperatively
(fiber-run-all fibers)

;; Fork parallel computation (HVM4 superposition)
(fork2 (expensive-a) (expensive-b))
```

## Documentation

- [Quickstart](docs/QUICKSTART.md) - Get running in 5 minutes (REPL, socket server, editor setup)
- [Getting Started](docs/GETTING_STARTED.md) - Installation and first steps
- [Quick Reference](docs/QUICK_REFERENCE.md) - Language overview and syntax
- [Pattern Matching Guide](docs/PATTERN_MATCHING_GUIDE.md) - Comprehensive pattern matching
- [Type System Guide](docs/TYPE_SYSTEM_GUIDE.md) - Types, inference, and dispatch
- [Effects and Handlers](docs/EFFECTS_AND_HANDLERS.md) - Algebraic effects tutorial
- [Standard Library](docs/STANDARD_LIBRARY.md) - All prelude functions
- [API Reference](docs/API_REFERENCE.md) - Complete function reference
- [Advanced Features](docs/ADVANCED_FEATURES.md) - Tower, macros, FFI
- [Syntax Specification](docs/SYNTAX.md) - Detailed syntax rules

## Architecture

OmniLisp compiles to HVM4, a parallel runtime based on interaction nets:

```
OmniLisp Source -> Parser -> AST -> HVM4 Constructors -> Reduction -> Result
                             |
                      Type Checking (optional)
                             |
                      Macro Expansion
```

Key components:
- **lib/runtime.hvm4** - Core evaluator, pattern matching, effects, types
- **lib/prelude.hvm4** - Standard library functions
- **clang/** - Parser and compiler (C)
- **docs/** - Documentation

## References

1. *Collapsing Towers of Interpreters* (Amin & Rompf, POPL 2018)
2. *Abstracting Control* (Danvy & Filinski, 1990)
3. *An Introduction to Algebraic Effects and Handlers* (Pretnar, 2015)
4. *Julia: A Fresh Approach to Numerical Computing* (Bezanson et al., 2017)

## License

MIT License - see [LICENSE](LICENSE) for details.

## Contributing

Contributions welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) first.
