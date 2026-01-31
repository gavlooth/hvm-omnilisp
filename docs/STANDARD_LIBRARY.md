# Standard Library Guide

This guide covers practical usage of OmniLisp's standard library (prelude). For a complete function reference, see [API Reference](API_REFERENCE.md).

## Collection Operations

OmniLisp has three main collection types with polymorphic operations that work across all of them.

### Lists, Arrays, and Dicts

```lisp
;; Lists - linked, good for head operations
(define nums '(1 2 3 4 5))

;; Arrays - indexed, good for random access
(define arr [1 2 3 4 5])

;; Dicts - key-value mappings
(define person #{"name" "Alice" "age" 30})
```

### Universal Operations

These work on all collection types:

```lisp
;; Length
(length '(1 2 3))     ;; -> 3
(length [1 2 3])      ;; -> 3
(length #{"a" 1})     ;; -> 1

;; Empty check
(empty? '())          ;; -> true
(empty? [])           ;; -> true
(empty? #{})          ;; -> true

;; First element
(first '(1 2 3))      ;; -> 1
(first [1 2 3])       ;; -> 1

;; Rest of collection
(rest '(1 2 3))       ;; -> '(2 3)
(rest [1 2 3])        ;; -> [2 3]
```

### Transformation

```lisp
;; map - transform each element
(map inc '(1 2 3))         ;; -> '(2 3 4)
(map inc [1 2 3])          ;; -> [2 3 4]
(map inc #{"a" 1 "b" 2})   ;; -> #{"a" 2 "b" 3}

;; filter - keep matching elements
(filter even? '(1 2 3 4 5)) ;; -> '(2 4)
(filter even? [1 2 3 4 5])  ;; -> [2 4]

;; reduce - fold into single value
(foldl + 0 '(1 2 3 4 5))   ;; -> 15
(foldr cons '() '(1 2 3))  ;; -> '(1 2 3)
```

### Slicing

```lisp
;; take - first n elements
(take 3 '(1 2 3 4 5))      ;; -> '(1 2 3)
(take 3 [1 2 3 4 5])       ;; -> [1 2 3]

;; drop - skip first n elements
(drop 2 '(1 2 3 4 5))      ;; -> '(3 4 5)
(drop 2 [1 2 3 4 5])       ;; -> [3 4 5]

;; slice - range of elements
(slice 1 4 [0 1 2 3 4 5])  ;; -> [1 2 3]
```

### Searching

```lisp
;; find - first match
(find even? '(1 3 4 5))    ;; -> #Som{4}
(find even? '(1 3 5 7))    ;; -> #Noth{}

;; find-index - position of match
(find-index even? '(1 3 4 5)) ;; -> #Som{2}

;; contains?
(contains? '(1 2 3) 2)     ;; -> true
(contains? [1 2 3] 5)      ;; -> false

;; index-of
(index-of '(a b c d) 'c)   ;; -> #Som{2}
```

### Combining

```lisp
;; append - join collections
(append '(1 2) '(3 4))     ;; -> '(1 2 3 4)
(append [1 2] [3 4])       ;; -> [1 2 3 4]

;; flatten - remove nesting
(flatten '((1 2) (3 (4 5)))) ;; -> '(1 2 3 4 5)

;; zip - pair elements
(zip '(1 2 3) '(a b c))    ;; -> '((1 a) (2 b) (3 c))

;; concat - multiple collections
(concat '(1 2) '(3 4) '(5 6)) ;; -> '(1 2 3 4 5 6)
```

### Grouping

```lisp
;; partition - split by predicate
(partition even? '(1 2 3 4 5 6))
;; -> ('(2 4 6) '(1 3 5))

;; group-by - group by key function
(group-by (lambda [x] (mod x 3)) '(1 2 3 4 5 6 7 8 9))
;; -> #{0 '(3 6 9) 1 '(1 4 7) 2 '(2 5 8)}

;; chunk - split into groups
(chunk 3 '(1 2 3 4 5 6 7 8))
;; -> '((1 2 3) (4 5 6) (7 8))
```

## List-Specific Operations

```lisp
;; cons - add to front
(cons 0 '(1 2 3))          ;; -> '(0 1 2 3)

;; head/tail - first/rest
(head '(1 2 3))            ;; -> 1
(tail '(1 2 3))            ;; -> '(2 3)

;; reverse
(reverse '(1 2 3))         ;; -> '(3 2 1)

;; sort
(sort '(3 1 4 1 5 9))      ;; -> '(1 1 3 4 5 9)
(sort-by length '("aaa" "b" "cc"))
;; -> '("b" "cc" "aaa")
```

## Array-Specific Operations

```lisp
;; get - index access (O(1))
(get [10 20 30] 1)         ;; -> 20

;; set - functional update
(set [10 20 30] 1 25)      ;; -> [10 25 30]

;; array-push - add to end
(array-push [1 2 3] 4)     ;; -> [1 2 3 4]

;; array-pop - remove from end
(array-pop [1 2 3])        ;; -> [1 2]
```

## Dict Operations

```lisp
;; get - value lookup
(get #{"a" 1 "b" 2} "a")   ;; -> 1
(get #{"a" 1} "c" 0)       ;; -> 0 (default)

;; assoc - add/update key
(assoc #{"a" 1} "b" 2)     ;; -> #{"a" 1 "b" 2}

;; dissoc - remove key
(dissoc #{"a" 1 "b" 2} "a") ;; -> #{"b" 2}

;; keys/values
(keys #{"a" 1 "b" 2})      ;; -> '("a" "b")
(values #{"a" 1 "b" 2})    ;; -> '(1 2)

;; merge - combine dicts
(merge #{"a" 1} #{"b" 2})  ;; -> #{"a" 1 "b" 2}

;; update - modify value
(update #{"count" 5} "count" inc)
;; -> #{"count" 6}

;; update-in - nested update
(update-in #{"user" #{"name" "Alice" "age" 30}} '("user" "age") inc)
;; -> #{"user" #{"name" "Alice" "age" 31}}
```

## Lazy Iterators

Lazy sequences compute values on demand:

```lisp
;; Create lazy ranges
(lazy-range 0 1000000)     ;; No memory used yet

;; Infinite sequences
(iterate inc 0)            ;; 0, 1, 2, 3, ...
(repeat 42)                ;; 42, 42, 42, ...
(cycle '(1 2 3))           ;; 1, 2, 3, 1, 2, 3, ...

;; Lazy operations
(-> (lazy-range 0 1000000)
    (lazy-filter even?)
    (lazy-map square)
    (lazy-take 10)
    (collect-list))
;; Only computes 10 values!
```

### Iterator Operations

```lisp
;; iter-next - get next value
(define it (lazy-range 0 10))
(iter-next it)             ;; -> #Som{0}
(iter-next it)             ;; -> #Som{1}

;; enumerate - add indices
(-> '(a b c)
    (iter-from-list)
    (enumerate)
    (collect-list))
;; -> '((0 a) (1 b) (2 c))

;; chain - concatenate iterators
(-> (chain (lazy-range 0 3) (lazy-range 10 13))
    (collect-list))
;; -> '(0 1 2 10 11 12)
```

### Realizing Iterators

```lisp
;; collect to list
(collect-list (lazy-range 0 5))  ;; -> '(0 1 2 3 4)

;; collect to array
(collect-array (lazy-range 0 5)) ;; -> [0 1 2 3 4]

;; into - type-directed collection
(into [] (lazy-range 0 5))       ;; -> [0 1 2 3 4]
(into '() (lazy-range 0 5))      ;; -> '(0 1 2 3 4)
```

## String Operations

```lisp
;; Concatenation
(str "Hello, " "World!")   ;; -> "Hello, World!"
(str 42)                   ;; -> "42"
(str '(1 2 3))             ;; -> "(1 2 3)"

;; Length
(string-length "hello")    ;; -> 5

;; Substring
(substring "hello" 1 4)    ;; -> "ell"

;; Access
(string-ref "hello" 0)     ;; -> #\h
(char-at "hello" 4)        ;; -> #\o

;; Case conversion
(string-upcase "hello")    ;; -> "HELLO"
(string-downcase "HELLO")  ;; -> "hello"

;; Trimming
(string-trim "  hello  ")  ;; -> "hello"
(string-trim-left "  hi") ;; -> "hi"
(string-trim-right "hi  ") ;; -> "hi"

;; Splitting/Joining
(string-split "a,b,c" ",") ;; -> '("a" "b" "c")
(string-join '("a" "b" "c") "-") ;; -> "a-b-c"

;; Predicates
(string-starts-with? "hello" "he") ;; -> true
(string-ends-with? "hello" "lo")   ;; -> true
(string-contains? "hello" "ll")    ;; -> true

;; Conversion
(string->list "abc")       ;; -> '(#\a #\b #\c)
(list->string '(#\a #\b))  ;; -> "ab"
(string->int "42")         ;; -> 42
(int->string 42)           ;; -> "42"
```

## Math Operations

### Basic Arithmetic

```lisp
(+ 1 2 3)                  ;; -> 6
(- 10 3)                   ;; -> 7
(* 2 3 4)                  ;; -> 24
(/ 10 3)                   ;; -> 3 (integer division)

(mod 17 5)                 ;; -> 2
(quot 17 5)                ;; -> 3
(rem 17 5)                 ;; -> 2
```

### Comparisons

```lisp
(= 1 1)                    ;; -> true
(!= 1 2)                   ;; -> true
(< 1 2)                    ;; -> true
(<= 1 1)                   ;; -> true
(> 2 1)                    ;; -> true
(>= 2 2)                   ;; -> true
```

### Math Functions

```lisp
(abs -5)                   ;; -> 5
(min 1 2 3)                ;; -> 1
(max 1 2 3)                ;; -> 3
(pow 2 10)                 ;; -> 1024
(sqrt 16)                  ;; -> 4.0
(exp 1)                    ;; -> 2.718...
(log 10)                   ;; -> 2.302...
```

### Trigonometry

```lisp
(sin 0)                    ;; -> 0.0
(cos 0)                    ;; -> 1.0
(tan 0)                    ;; -> 0.0
(asin 1)                   ;; -> 1.570... (pi/2)
(acos 0)                   ;; -> 1.570...
(atan 1)                   ;; -> 0.785... (pi/4)
```

### Integer Math

```lisp
(gcd 12 18)                ;; -> 6
(lcm 4 6)                  ;; -> 12
(even? 4)                  ;; -> true
(odd? 5)                   ;; -> true
```

## I/O Operations

### Console

```lisp
;; Output
(print "no newline")
(println "with newline")

;; Input
(read-line _)              ;; Read line from stdin
```

### Files

```lisp
;; Read entire file
(read-file "path.txt")     ;; -> "file contents"

;; Write file
(write-file "path.txt" "content")

;; Read lines
(read-lines "path.txt")    ;; -> '("line1" "line2" ...)

;; Append to file
(append-file "path.txt" "more content")

;; Check existence
(file-exists? "path.txt")  ;; -> true/false
```

### Directories

```lisp
;; List directory
(list-dir "/path")         ;; -> '("file1" "file2" ...)

;; Create directory
(make-dir "/path/new")

;; Current directory
(current-dir)              ;; -> "/current/path"

;; Find files
(find-files "/src" (lambda [f] (string-ends-with? f ".ol")))
```

## Regular Expressions (Pika)

OmniLisp uses Pika-based regex (no backtracking):

```lisp
;; Test if pattern matches
(re-test "[0-9]+" "abc123")     ;; -> true

;; Full match
(re-match "[0-9]+" "123")      ;; -> "123"

;; Find first match
(re-find "[0-9]+" "abc123def") ;; -> #Som{(3 "123")}

;; Find all matches
(re-find-all "[0-9]+" "a1b22c333")
;; -> '((1 "1") (3 "22") (6 "333"))

;; Replace
(re-replace "[0-9]" "X" "a1b2c3")     ;; -> "aXb2c3"
(re-replace-all "[0-9]" "X" "a1b2c3") ;; -> "aXbXcX"

;; Split
(re-split "[,;]" "a,b;c")      ;; -> '("a" "b" "c")
```

## JSON

```lisp
;; Parse JSON string
(json-parse "{\"name\": \"Alice\", \"age\": 30}")
;; -> #{"name" "Alice" "age" 30}

(json-parse "[1, 2, 3]")
;; -> [1 2 3]

;; Stringify to JSON
(json-stringify #{"items" '(1 2 3) "active" true})
;; -> "{\"items\":[1,2,3],\"active\":true}"
```

## Date/Time

```lisp
;; Current time
(datetime-now)

;; Format
(datetime-format (datetime-now) "%Y-%m-%d %H:%M:%S")
;; -> "2024-01-15 14:30:45"

;; Parse
(datetime-parse "2024-01-15" "%Y-%m-%d")

;; Arithmetic
(datetime-add-days (datetime-now) 7)
(datetime-add-hours (datetime-now) 24)

;; Difference
(datetime-diff dt1 dt2)    ;; -> seconds between

;; Components
(datetime-year dt)
(datetime-month dt)
(datetime-day dt)
(datetime-hour dt)
(datetime-minute dt)
(datetime-second dt)
```

## Networking

### HTTP

```lisp
;; Simple GET request
(http-get "example.com" 80 "/api/data")

;; With headers
(http-request "GET" "https://api.example.com/users"
              #{"Authorization" "Bearer token"})
```

### TCP

```lisp
;; Client
(let [sock (tcp-connect "localhost" 8080)]
  (tcp-send sock "Hello")
  (let [response (tcp-recv sock 1024)]
    (tcp-close sock)
    response))

;; Server
(tcp-serve 8080
  (lambda [client]
    (let [request (tcp-recv client 1024)]
      (tcp-send client "HTTP/1.1 200 OK\r\n\r\nHello!")
      (tcp-close client))))
```

## Functional Utilities

### Function Composition

```lisp
;; compose - right to left
(define add1-then-double (compose double inc))
(add1-then-double 5)       ;; -> 12

;; pipe - left to right
(-> 5
    inc
    double
    str)                   ;; -> "12"

;; partial application
(define add5 (partial + 5))
(add5 3)                   ;; -> 8
```

### Predicates

```lisp
;; complement - negate predicate
(define not-empty? (complement empty?))
(not-empty? '(1 2))        ;; -> true

;; every? - all match
(every? even? '(2 4 6))    ;; -> true

;; any? - some match
(any? even? '(1 3 4 5))    ;; -> true

;; none? - none match
(none? even? '(1 3 5))     ;; -> true
```

### Control Flow

```lisp
;; identity
(identity 42)              ;; -> 42

;; constantly - always return same value
(define always-5 (constantly 5))
(always-5 "ignored")       ;; -> 5

;; when-let - bind and test
(when-let [x (find even? items)]
  (process x))

;; if-let - bind and branch
(if-let [x (find even? items)]
  (process x)
  (handle-none))
```

## Type Utilities

```lisp
;; Type checking
(type? 42 Int)             ;; -> true
(type? "hi" String)        ;; -> true

;; Get type
(type-of 42)               ;; -> Int
(type-of "hi")             ;; -> String

;; Predicates
(int? 42)                  ;; -> true
(string? "hi")             ;; -> true
(list? '(1 2))             ;; -> true
(array? [1 2])             ;; -> true
(dict? #{"a" 1})           ;; -> true
(function? inc)            ;; -> true
(nothing? nothing)         ;; -> true
```

## Further Reading

- [API Reference](API_REFERENCE.md) - Complete function reference
- [Getting Started](GETTING_STARTED.md) - Tutorial
- [Pattern Matching Guide](PATTERN_MATCHING_GUIDE.md) - Collection patterns
- [Quick Reference](QUICK_REFERENCE.md) - Language overview
