# OmniLisp API Reference

Complete reference for all prelude functions and runtime operations.

---

## Table of Contents

1. [Core Functions](#core-functions)
2. [List Operations](#list-operations)
3. [Array Operations](#array-operations)
4. [Dict Operations](#dict-operations)
5. [Higher-Order Functions](#higher-order-functions)
6. [Lazy Iterators](#lazy-iterators)
7. [String Operations](#string-operations)
8. [Math Functions](#math-functions)
9. [Float Operations](#float-operations)
10. [Type System](#type-system)
11. [Pattern Matching](#pattern-matching)
12. [Effects and Handlers](#effects-and-handlers)
13. [Fibers and Concurrency](#fibers-and-concurrency)
14. [I/O Operations](#io-operations)
15. [File System](#file-system)
16. [Networking](#networking)
17. [JSON](#json)
18. [DateTime](#datetime)
19. [Regex](#regex)
20. [Pika Grammar](#pika-grammar)
21. [Macros](#macros)
22. [Tower of Interpreters](#tower-of-interpreters)
23. [Testing](#testing)
24. [Debugging](#debugging)

---

## Core Functions

### Arithmetic

```lisp
(+ a b)       ;; Addition
(- a b)       ;; Subtraction
(* a b)       ;; Multiplication
(/ a b)       ;; Division
(mod a b)     ;; Modulo
(div a b)     ;; Integer division
```

### Comparison

```lisp
(= a b)       ;; Equality
(!= a b)      ;; Inequality
(< a b)       ;; Less than
(> a b)       ;; Greater than
(<= a b)      ;; Less than or equal
(>= a b)      ;; Greater than or equal
(eq? a b)     ;; Reference equality
```

### Logic

```lisp
(and a b)     ;; Logical AND
(or a b)      ;; Logical OR
(not a)       ;; Logical NOT
```

### Identity and Constants

```lisp
(identity x)       ;; Returns x unchanged
(const x y)        ;; Returns x, ignores y
(nothing)          ;; Returns #Noth{}
```

---

## List Operations

### Construction

```lisp
(cons h t)              ;; Prepend h to list t
(list a b c ...)        ;; Create list from elements
(make-list n val)       ;; Create list of n copies of val
```

### Access

```lisp
(head xs)               ;; First element (alias: first, car)
(tail xs)               ;; Rest of list (alias: rest, cdr)
(nth xs n)              ;; Element at index n
(last xs)               ;; Last element
```

### Query

```lisp
(length xs)             ;; Number of elements
(nil? xs)               ;; True if empty
(empty? xs)             ;; True if empty
(contains? xs x)        ;; True if x in xs
(member? x xs)          ;; True if x in xs
```

### Transformation

```lisp
(reverse xs)            ;; Reverse list
(append xs ys)          ;; Concatenate lists
(flatten xs)            ;; Flatten nested lists
(intersperse sep xs)    ;; Insert sep between elements
(intercalate sep xss)   ;; Flatten with separator
```

### Slicing

```lisp
(take n xs)             ;; First n elements
(drop n xs)             ;; All but first n
(take-while pred xs)    ;; Take while predicate holds
(drop-while pred xs)    ;; Drop while predicate holds
(split-at n xs)         ;; Split at index n -> [left, right]
```

### Grouping

```lisp
(partition pred xs)     ;; Split by predicate -> [true-list, false-list]
(group-by f xs)         ;; Group by key function -> dict
(span pred xs)          ;; Split at first false -> [prefix, suffix]
(break pred xs)         ;; Split at first true -> [prefix, suffix]
```

### Deduplication

```lisp
(nub xs)                ;; Remove duplicates
(nub-by eq-fn xs)       ;; Remove duplicates with custom equality
```

---

## Array Operations

Arrays support the same operations as lists, with optimized random access:

```lisp
[1 2 3 4 5]             ;; Array literal
(array-get arr i)       ;; Get element at index
(array-set arr i v)     ;; Return new array with element set
(array-length arr)      ;; Length (O(1))
(array->list arr)       ;; Convert to list
(list->array xs)        ;; Convert from list
```

---

## Dict Operations

### Construction

```lisp
#{"key" val ...}        ;; Dict literal
(dict pairs)            ;; Create from list of [key val] pairs
(empty-dict)            ;; Empty dict
```

### Access

```lisp
(get dict key)          ;; Get value for key
(get-in dict path)      ;; Get nested value
(keys dict)             ;; List of keys
(values dict)           ;; List of values
(entries dict)          ;; List of [key val] pairs
```

### Modification (functional)

```lisp
(assoc dict key val)    ;; Return dict with key set
(assoc-in dict path val);; Return dict with nested key set
(dissoc dict key)       ;; Return dict without key
(update dict key f)     ;; Return dict with (f (get dict key))
(update-in dict path f) ;; Nested update
(merge d1 d2)           ;; Merge dicts (d2 overrides)
```

### Query

```lisp
(dict-contains? d key)  ;; True if key exists
(dict-empty? d)         ;; True if empty
(dict-size d)           ;; Number of entries
```

---

## Higher-Order Functions

### Mapping

```lisp
(map f xs)              ;; Apply f to each element
(map-indexed f xs)      ;; f receives (index, element)
(for-each f xs)         ;; Apply f for side effects, return nothing
```

### Filtering

```lisp
(filter pred xs)        ;; Keep elements where (pred x) is true
(remove pred xs)        ;; Remove elements where (pred x) is true
(keep f xs)             ;; Keep non-nothing results of (f x)
```

### Folding

```lisp
(foldl f init xs)       ;; Left fold: f(f(f(init, x1), x2), x3)
(foldr f init xs)       ;; Right fold: f(x1, f(x2, f(x3, init)))
(reduce f xs)           ;; Fold without initial value
(scan f init xs)        ;; List of intermediate fold values
```

### Searching

```lisp
(find pred xs)          ;; First element satisfying pred, or #Noth{}
(find-index pred xs)    ;; Index of first match, or #Noth{}
(index-of x xs)         ;; Index of x, or #Noth{}
```

### Aggregation

```lisp
(all? pred xs)          ;; True if all satisfy pred
(any? pred xs)          ;; True if any satisfies pred
(none? pred xs)         ;; True if none satisfy pred
(count pred xs)         ;; Count of elements satisfying pred
(sum xs)                ;; Sum of numbers
(product xs)            ;; Product of numbers
```

### Sorting

```lisp
(sort xs)               ;; Sort ascending
(sort-by f xs)          ;; Sort by key function
(sort-asc xs)           ;; Sort ascending (explicit)
(sort-desc xs)          ;; Sort descending
(minimum xs)            ;; Smallest element
(maximum xs)            ;; Largest element
(minimum-by f xs)       ;; Smallest by key function
(maximum-by f xs)       ;; Largest by key function
```

### Combining

```lisp
(zip xs ys)             ;; Pair elements: '([x1 y1] [x2 y2] ...)
(zip-with f xs ys)      ;; Combine with function
(unzip pairs)           ;; Split pairs into two lists
```

### Composition

```lisp
(compose f g)           ;; (compose f g) x = f(g(x))
(pipe x f g h)          ;; h(g(f(x))) - same as (|> x f g h)
(|> x f g h)            ;; Threading macro
(flip f)                ;; (flip f) a b = f b a
(curry f)               ;; Convert multi-arg to curried
(uncurry f)             ;; Convert curried to multi-arg
(partial f arg)         ;; Partially apply first argument
(partial2 f a b)        ;; Partially apply two arguments
```

---

## Lazy Iterators

### Creation

```lisp
(lazy-range start end)       ;; Lazy range [start, end)
(lazy-range1 n)              ;; Lazy range [0, n)
(lazy-range2 start end step) ;; Lazy range with step
(iterate f x)                ;; Infinite: x, f(x), f(f(x)), ...
(lazy-repeat x)              ;; Infinite repetition of x
(cycle xs)                   ;; Infinite cycle through xs
```

### Transformation

```lisp
(lazy-map f iter)            ;; Lazy map
(lazy-filter pred iter)      ;; Lazy filter
(lazy-take n iter)           ;; Take first n (lazy)
(lazy-drop n iter)           ;; Drop first n (lazy)
(lazy-take-while pred iter)  ;; Take while predicate holds
(lazy-drop-while pred iter)  ;; Drop while predicate holds
(lazy-zip iter1 iter2)       ;; Lazy zip
(lazy-zip-with f i1 i2)      ;; Lazy zip with function
(chain iter1 iter2)          ;; Concatenate iterators
(lazy-flatten iter)          ;; Flatten nested iterators
(enumerate iter)             ;; Add indices: [0 x] [1 y] ...
```

### Realization

```lisp
(collect-list iter)          ;; Realize to list
(to-list iter)               ;; Alias for collect-list
(iter-fold f init iter)      ;; Fold over iterator
(iter-reduce f iter)         ;; Reduce iterator
(iter-count iter)            ;; Count elements
(iter-sum iter)              ;; Sum elements
(iter-find pred iter)        ;; Find first matching
(iter-any pred iter)         ;; Any element matches?
(iter-all pred iter)         ;; All elements match?
(iter-nth n iter)            ;; Get nth element
(iter-last iter)             ;; Get last element
```

### Protocol

```lisp
(iter-next iter)             ;; Get next: #Some{val, new-iter} or #None{}
(iter-done? iter)            ;; Check if exhausted
```

---

## String Operations

```lisp
(str a b c ...)         ;; Concatenate to string
(string-length s)       ;; Length
(string-char-at s i)    ;; Character at index
(string-slice s i j)    ;; Substring [i, j)
(string-concat a b)     ;; Concatenate two strings
(string->list s)        ;; Convert to character list
(list->string cs)       ;; Convert from character list
(split-by sep s)        ;; Split string by separator
(split-lines s)         ;; Split by newlines
(trim s)                ;; Remove leading/trailing whitespace
(starts-with? s prefix) ;; Check prefix
(ends-with? s suffix)   ;; Check suffix
(char-at s i)           ;; Character at index (alias)
```

---

## Math Functions

### Basic

```lisp
(abs x)                 ;; Absolute value
(sign x)                ;; -1, 0, or 1
(min a b)               ;; Minimum
(max a b)               ;; Maximum
(clamp lo hi x)         ;; Clamp x to [lo, hi]
```

### Integer

```lisp
(mod a b)               ;; Modulo
(div a b)               ;; Integer division
(gcd a b)               ;; Greatest common divisor
(lcm a b)               ;; Least common multiple
(even? x)               ;; True if even
(odd? x)                ;; True if odd
```

### Trigonometry

```lisp
(sin x) (cos x) (tan x)     ;; Basic trig
(asin x) (acos x) (atan x)  ;; Inverse trig
(atan2 y x)                 ;; Two-argument arctangent
(sinh x) (cosh x) (tanh x)  ;; Hyperbolic
```

### Exponential

```lisp
(exp x)                 ;; e^x
(log x) (ln x)          ;; Natural logarithm
(log10 x)               ;; Base-10 logarithm
(log2 x)                ;; Base-2 logarithm
(pow x n)               ;; x^n
(sqrt x)                ;; Square root
(cbrt x)                ;; Cube root
(hypot x y)             ;; sqrt(x^2 + y^2)
```

### Rounding

```lisp
(floor x)               ;; Round down
(ceil x)                ;; Round up
(round x)               ;; Round to nearest
(truncate x)            ;; Round toward zero
```

### Constants

```lisp
@pi                     ;; 3.14159...
@euler                  ;; 2.71828...
@tau                    ;; 2 * pi
@phi                    ;; Golden ratio
@sqrt-2                 ;; sqrt(2)
```

### Random

```lisp
(rand_next seed)        ;; Next random from seed
(rand_int max seed)     ;; Random int in [0, max)
```

---

## Float Operations

OmniLisp uses fixed-point representation: `#Fix{hi, lo, scale}` where value = mantissa / 10^scale.

### Construction

```lisp
(float mantissa scale)  ;; Create float: (float 314159 5) = 3.14159
(int->float n)          ;; Integer to float
(float-ratio num den)   ;; Create from fraction
```

### Arithmetic

```lisp
(float+ a b)            ;; Addition
(float- a b)            ;; Subtraction
(float* a b)            ;; Multiplication
(float/ a b)            ;; Division
(float-negate x)        ;; Negation
(float-abs x)           ;; Absolute value
```

### Comparison

```lisp
(float= a b)            ;; Equality
(float< a b)            ;; Less than
(float> a b)            ;; Greater than
(float<= a b)           ;; Less than or equal
(float>= a b)           ;; Greater than or equal
(float-compare a b)     ;; Returns -1, 0, or 1
```

### Rounding

```lisp
(float-round x places)  ;; Round to n decimal places
(float-floor x)         ;; Round down
(float-ceil x)          ;; Round up
(float-truncate x)      ;; Truncate toward zero
```

### Conversion

```lisp
(float->int x)          ;; Convert to integer
(float->string x)       ;; Format as string
(string->float s)       ;; Parse string to float
```

### Polymorphic (work on Int or Float)

```lisp
(num+ a b) (num- a b) (num* a b) (num/ a b)
```

---

## Type System

### Type Checking

```lisp
(type-of val)           ;; Get type descriptor #TDsc{...}
(type-name val)         ;; Get type name as symbol
(type-check val type)   ;; Check if val matches type
(subtype? a b)          ;; Check if a is subtype of b
```

### Type Predicates

```lisp
(int? x)                ;; Is integer?
(float? x) (is-float? x);; Is float?
(string? x) (str? x)    ;; Is string?
(list? x)               ;; Is list?
(array? x)              ;; Is array?
(dict? x)               ;; Is dict?
(symbol? x)             ;; Is symbol?
(function? x)           ;; Is callable?
(nil? x)                ;; Is nil/empty?
(nothing? x)            ;; Is #Noth{}?
(some? x)               ;; Is #Som{...}?
```

### Type Construction

```lisp
(get-type name)              ;; Get built-in type by name
(union-type name types)      ;; Create union type
(intersection-type types)    ;; Create intersection type
(value-type val)             ;; Create singleton type
(function-type args ret)     ;; Create function type
(param-type base params)     ;; Create parameterized type
(type-var name)              ;; Create type variable
(bounded-type-var name bound);; Create bounded type variable
```

### Type Aliases

```lisp
(deftype-alias name target)  ;; Register type alias
(lookup-type-alias name)     ;; Lookup alias
```

### Type Inference

```lisp
(infer-type expr)            ;; Infer type of expression
(typed-lambda [x] body)      ;; Create lambda with inferred type
(closure-type clo)           ;; Get inferred type of closure
```

---

## Pattern Matching

### Pattern Constructors

```lisp
(wildcard)                   ;; Match anything, bind nothing: _
(bind-pattern idx)           ;; Bind to de Bruijn index
(lit-pattern val)            ;; Match literal value
(ctr-pattern name args)      ;; Match constructor
(or-pattern left right)      ;; Match either pattern
(spread-pattern head tail)   ;; Match head + rest: (h .. t)
(list-pattern pats)          ;; Match exact list structure
(dict-pattern entries)       ;; Match dict entries
(pred-pattern inner pred)    ;; Match with predicate guard
(view-pattern fn inner)      ;; Apply function, match result
```

---

## Effects and Handlers

### Effect Operations

```lisp
(perform tag payload)        ;; Raise an effect
(handle body handlers...)    ;; Handle effects in body
```

### Handler Syntax

```lisp
(handle
  body
  (effect-name [payload resume]
    handler-body))
```

### Delimited Continuations

```lisp
(reset body)                 ;; Delimit continuation
(shift k body)               ;; Capture continuation as k
(control k body)             ;; Capture without auto-resume
```

---

## Fibers and Concurrency

### Fiber Creation

```lisp
(fiber-new thunk)            ;; Create fiber from thunk
(fiber-spawn body)           ;; Spawn fiber running body
```

### Fiber Control

```lisp
(fiber-run fiber)            ;; Run fiber until yield/done
(fiber-resume fiber)         ;; Resume suspended fiber
(fiber-resume-unit fiber)    ;; Resume with unit value
(yield val)                  ;; Yield value from fiber
(yield-unit)                 ;; Yield unit
```

### Fiber State

```lisp
(fiber-done? fiber)          ;; Is fiber complete?
(fiber-suspended? fiber)     ;; Is fiber suspended?
(fiber-running? fiber)       ;; Is fiber running?
(fiber-result fiber)         ;; Get result (if done)
(fiber-mailbox fiber)        ;; Get fiber's mailbox
```

### Scheduling

```lisp
(fiber-run-all fibers)       ;; Run all fibers to completion
(fiber-run-all-n fibers n)   ;; Run with step limit
```

### Parallelism

```lisp
(fork2 a b)                  ;; Fork two computations (HVM4 SUP)
(choice opts)                ;; Nondeterministic choice
```

---

## I/O Operations

### Console

```lisp
(print s)                    ;; Print without newline
(println s)                  ;; Print with newline
(read_line _)                ;; Read line from stdin
(putchar c)                  ;; Write character
(getchar _)                  ;; Read character
```

### Files

```lisp
(read_file path)             ;; Read entire file
(write_file path content)    ;; Write file (overwrite)
(append_file path content)   ;; Append to file
(read_lines path)            ;; Read file as list of lines
(file_open path mode)        ;; Open file handle
(file_close handle)          ;; Close file handle
(file_exists? path)          ;; Check if file exists
(delete_file path)           ;; Delete file
(rename_file old new)        ;; Rename file
```

### Paths

```lisp
(path_join a b)              ;; Join path components
(dirname path)               ;; Directory part
(basename path)              ;; File name part
(extension path)             ;; File extension
(getcwd _)                   ;; Current directory
(chdir path)                 ;; Change directory
```

### Environment

```lisp
(getenv name)                ;; Get environment variable
(setenv name value)          ;; Set environment variable
```

### Timing

```lisp
(time _)                     ;; Current Unix timestamp
(sleep seconds)              ;; Sleep for seconds
```

---

## File System

```lisp
(list-dir path)              ;; List directory entries
(list-dir-paths path)        ;; List with full paths
(dir? path)                  ;; Is directory?
(walk-dir path callback)     ;; Recursively walk directory
(find-files path predicate)  ;; Find files matching predicate
```

---

## Networking

### Address

```lisp
(make-addr ip port)          ;; Create address
(localhost port)             ;; 127.0.0.1:port
(any-addr port)              ;; 0.0.0.0:port
(resolve-host hostname)      ;; DNS lookup
```

### TCP Client

```lisp
(tcp-connect host port)      ;; Connect to server
(tcp-send sock data)         ;; Send data
(tcp-recv sock max-bytes)    ;; Receive data
(tcp-send-all sock data)     ;; Send all data
(tcp-recv-all sock)          ;; Receive until close
(tcp-recv-line sock)         ;; Receive until newline
```

### TCP Server

```lisp
(tcp-server port)            ;; Create server socket
(socket-accept sock)         ;; Accept connection
(tcp-serve port handler)     ;; Accept loop with handler
(tcp-serve-n port handler n) ;; Serve n connections
```

### Socket Control

```lisp
(socket-close sock)          ;; Close socket
(socket-setopt sock opt val) ;; Set socket option
(socket-reuse sock)          ;; Enable address reuse
(socket-timeout sock ms)     ;; Set timeout
```

### HTTP

```lisp
(http-get host port path)    ;; Simple HTTP GET
```

### Async I/O (with fibers)

```lisp
(async-connect host port)    ;; Non-blocking connect
(async-send sock data)       ;; Non-blocking send
(async-recv sock max)        ;; Non-blocking receive
(async-accept sock)          ;; Non-blocking accept
(parallel-fetch requests)    ;; Parallel HTTP requests
```

---

## JSON

### Parsing

```lisp
(json-parse str)             ;; Parse JSON string
(json-type val)              ;; Get JSON type as symbol
(json-serializable? val)     ;; Can value be serialized?
```

### Stringification

```lisp
(json-stringify val)         ;; Convert to JSON string
```

### Type Predicates

```lisp
(json-object? val)           ;; Is JSON object?
(json-array? val)            ;; Is JSON array?
(json-string? val)           ;; Is JSON string?
(json-number? val)           ;; Is JSON number?
(json-boolean? val)          ;; Is JSON boolean?
(json-null? val)             ;; Is JSON null?
```

### Convenience

```lisp
(json-get-key json key)      ;; Parse and get key
(json-get-path json path)    ;; Parse and get nested path
```

---

## DateTime

### Construction

```lisp
(datetime y m d h min s)     ;; Create datetime
(datetime-tz y m d h min s tz) ;; With timezone offset
(date y m d)                 ;; Date only
(time-of-day h m s)          ;; Time only
```

### Current Time

```lisp
(datetime-now)               ;; Local time
(datetime-now-utc)           ;; UTC time
(date-today)                 ;; Today's date
```

### Components

```lisp
(datetime-year dt)           ;; Year
(datetime-month dt)          ;; Month (1-12)
(datetime-day dt)            ;; Day (1-31)
(datetime-hour dt)           ;; Hour (0-23)
(datetime-minute dt)         ;; Minute (0-59)
(datetime-second dt)         ;; Second (0-59)
(datetime-tz-offset dt)      ;; Timezone offset in seconds
(datetime-day-of-week dt)    ;; 0=Sunday, 6=Saturday
```

### Arithmetic

```lisp
(datetime-add-seconds dt n)  ;; Add seconds
(datetime-add-minutes dt n)  ;; Add minutes
(datetime-add-hours dt n)    ;; Add hours
(datetime-add-days dt n)     ;; Add days
(datetime-add-weeks dt n)    ;; Add weeks
(datetime-diff dt1 dt2)      ;; Difference in seconds
(datetime-diff-days dt1 dt2) ;; Difference in days
```

### Comparison

```lisp
(datetime-before? a b)       ;; Is a before b?
(datetime-after? a b)        ;; Is a after b?
(datetime-equal? a b)        ;; Are they equal?
(datetime-compare a b)       ;; Returns -1, 0, or 1
```

### Formatting

```lisp
(datetime-format dt fmt)     ;; Format with custom string
(datetime-format-iso dt)     ;; ISO 8601 format
(datetime-format-date dt)    ;; YYYY-MM-DD
(datetime-format-time dt)    ;; HH:MM:SS
```

### Calendar

```lisp
(datetime-day-name dt)       ;; "Sunday", "Monday", etc.
(datetime-month-name dt)     ;; "January", "February", etc.
(leap-year? year)            ;; Is leap year?
(days-in-month year month)   ;; Days in month
```

### Epoch

```lisp
(epoch-to-datetime-local ts) ;; Unix timestamp to local
(epoch-to-datetime-utc ts)   ;; Unix timestamp to UTC
(datetime-to-epoch dt)       ;; DateTime to Unix timestamp
```

---

## Regex

OmniLisp uses Pika-based regex with no backtracking (guaranteed O(n*m) performance).

### Matching

```lisp
(re-match pattern str)       ;; Match entire string
(re-match-start pattern str) ;; Match at start
(re-test pattern str)        ;; Match anywhere
```

### Searching

```lisp
(re-find pattern str)        ;; Find first match: #Som{[pos, match]}
(re-find-all pattern str)    ;; Find all matches: '([pos match] ...)
```

### Transformation

```lisp
(re-replace pattern repl str)       ;; Replace all matches
(re-replace-first pattern repl str) ;; Replace first match
(re-split pattern str)              ;; Split by pattern
```

### Supported Syntax

- `.` - any character
- `*` `+` `?` - quantifiers (greedy)
- `[abc]` `[^abc]` `[a-z]` - character classes
- `|` - alternation
- `()` - grouping
- `^` `$` - anchors
- `\d` `\D` `\w` `\W` `\s` `\S` - escape classes
- `\n` `\t` `\r` `\\` - escape sequences

### Convenience

```lisp
(all-digits? str)            ;; String is all digits?
(identifier? str)            ;; Valid identifier?
(has-whitespace? str)        ;; Contains whitespace?
(extract-numbers str)        ;; Extract all numbers
(split-whitespace str)       ;; Split by whitespace
(trim str)                   ;; Remove leading/trailing whitespace
```

---

## Pika Grammar

Pika is a bottom-up parsing algorithm (NOT PEG) that handles left recursion natively.

### Grammar Construction

```lisp
(grammar rules start)        ;; Create grammar
(rule name pattern)          ;; Create named rule
```

### Combinators

```lisp
(g-lit str)                  ;; Match literal string
(g-ref name)                 ;; Reference rule by name
(g-seq a b)                  ;; Sequence: a then b
(g-alt a b)                  ;; Alternation: a or b
(g-star g)                   ;; Zero or more
(g-plus g)                   ;; One or more
(g-opt g)                    ;; Optional (zero or one)
(g-not g)                    ;; Negative lookahead
(g-and g)                    ;; Positive lookahead
(g-range lo hi)              ;; Character range
```

### Common Patterns

```lisp
@g-digit                     ;; [0-9]
@g-letter                    ;; [a-zA-Z]
@g-alnum                     ;; [a-zA-Z0-9]
@g-ws                        ;; Whitespace
@g-ident                     ;; Identifier
@g-quoted                    ;; Quoted string "..."
@g-integer                   ;; -?[0-9]+
```

### Parsing

```lisp
(pika-parse grammar rule str);; Parse string with rule
(parse-success? result)      ;; Did parse succeed?
(parse-matched result)       ;; Get matched value
(parse-end result)           ;; Get end position
(parse-error result)         ;; Get error info
(format-parse-result result) ;; Format for display
```

---

## Macros

### Definition

```lisp
(define [syntax name]
  [pattern1 template1]
  [pattern2 template2]
  ...)
```

### Ellipsis

```lisp
(define [syntax let*]
  [(let* () body) body]
  [(let* ([v e] rest ...) body)
   (let [v e] (let* (rest ...) body))])
```

### Hygiene

```lisp
(gensym)                     ;; Generate unique symbol
(gensym? sym)                ;; Is symbol a gensym?
(fresh-mark)                 ;; Create hygiene mark
(add-mark sym mark)          ;; Add mark to symbol
(sym-eq-hygienic a b)        ;; Compare with hygiene
(expand-hygienic mac expr)   ;; Hygienic expansion
(hygiene-check expr bindings);; Verify no capture
```

---

## Tower of Interpreters

### Code Values

```lisp
(code expr)                  ;; Wrap expression as code
(code-unwrap code)           ;; Get expression from code
(code-exec code)             ;; Execute code value
(code-value? x)              ;; Is code value?
```

### Reflection

```lisp
(reflect val)                ;; Value to code representation
(reify code)                 ;; Execute code to produce value
```

### Staging

```lisp
(lift val)                   ;; Lift value to next stage
(run code)                   ;; Run at parent level
(stage level expr)           ;; Stage at specific level
(splice code)                ;; Splice code into current stage
(meta-level)                 ;; Current tower level
(eval-meta expr)             ;; Evaluate at meta-level
```

### Code Construction

```lisp
(make-lit n)                 ;; #Lit{n}
(make-var i)                 ;; #Var{i}
(make-lam body)              ;; #Lam{body}
(make-app fn arg)            ;; #App{fn, arg}
(make-let val body)          ;; #Let{val, body}
(make-if c t e)              ;; #If{c, t, e}
(make-add a b)               ;; #Add{a, b}
```

### Partial Evaluation

```lisp
(pe-optimize code)           ;; Optimize code
(pe-optimize-n code n)       ;; n optimization passes
(pe-const? code)             ;; Is constant?
(pe-get-const code)          ;; Get constant value
(pe-code-size code)          ;; Code size metric
(pe-code-eq a b)             ;; Code equality
```

---

## Testing

### Test Definition

```lisp
(deftest "name" body)        ;; Define a test
(defsuite "name" tests)      ;; Define a test suite
```

### Running Tests

```lisp
(run-tests tests)            ;; Run tests, get summary
(test-and-report tests)      ;; Run and print results
(tests-passed? summary)      ;; Did all tests pass?
(format-test-results summary);; Format for display
```

### Assertions

```lisp
(assert cond msg)            ;; Assert condition
(assert= a b)                ;; Assert equality
(assert-not= a b)            ;; Assert inequality
(assert-pred pred val msg)   ;; Assert predicate holds
(assert-type val type)       ;; Assert type
(assert-true val)            ;; Assert truthy
(assert-false val)           ;; Assert falsy
(assert-nil val)             ;; Assert nil
(assert-not-nil val)         ;; Assert not nil
(assert-error expr)          ;; Assert throws error
```

### Result Inspection

```lisp
(test-count summary)         ;; Total test count
(passed-count summary)       ;; Passed count
(failed-count summary)       ;; Failed count
(error-count summary)        ;; Error count
(merge-results s1 s2)        ;; Combine summaries
```

---

## Debugging

### Inspection

```lisp
(inspect val)                ;; Get structure info
(type-of val)                ;; Get type descriptor
(type-name val)              ;; Get type name
```

### Tracing

```lisp
(trace label expr)           ;; Print label => result
(timed expr)                 ;; Return [result, elapsed_ns]
(debug expr)                 ;; Wrap with debug info
```

### Pretty Printing

```lisp
(pp val)                     ;; Pretty print value
(tap val)                    ;; Print and return val
(tap-with f val)             ;; Print (f val), return val
```

### Benchmarking

```lisp
(bench expr)                 ;; Benchmark expression
(bench-warmup expr n)        ;; Warmup then benchmark
```

### Stack Traces

```lisp
(stack-trace)                ;; Get current stack trace
(format-stack trace)         ;; Format stack trace
(format-frame frame)         ;; Format single frame
(make-frame name file line col) ;; Create frame
(print-stack trace)          ;; Print to console
(with-stack-trace body)      ;; Wrap with stack tracing
```

---

## Index

Quick reference for finding functions by category:

**Collections**: map, filter, foldl, foldr, reduce, take, drop, reverse, sort, find, partition, group-by, zip

**Strings**: str, string-length, string-slice, split-by, trim, starts-with?, ends-with?

**Math**: +, -, *, /, mod, abs, sqrt, pow, sin, cos, log, floor, ceil, round

**I/O**: print, println, read_file, write_file, list-dir

**Network**: tcp-connect, tcp-send, tcp-recv, http-get

**JSON**: json-parse, json-stringify

**DateTime**: datetime-now, datetime-format, datetime-add-days

**Regex**: re-match, re-find, re-replace, re-split

**Types**: type-of, type-check, subtype?, int?, string?, list?

**Effects**: perform, handle, reset, shift

**Fibers**: fiber-spawn, yield, fiber-run-all

**Testing**: deftest, assert, assert=, run-tests
