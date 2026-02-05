# Phase 5.1: Standard Library

**Status:** Design Phase  
**Date:** 2026-02-05  
**Depends on:** Phases 3-4 (Core Language)

## Overview

The standard library provides essential functions and data structures organized into modules:
- **Prelude** (auto-imported basics)
- **Collections** (list, map, set, vector)
- **String** operations
- **Math** functions
- **IO** operations
- **Option/Result** types

## Organization

```
omnilisp.core/
├── prelude      # Auto-imported
├── list         # List operations
├── string       # String manipulation  
├── math         # Mathematical functions
├── option       # Optional values
├── result       # Error handling
└── io           # I/O operations

omnilisp.data/
├── map          # Persistent maps
├── set          # Persistent sets
├── vector       # Persistent vectors
└── queue        # Queue structures
```

## Prelude (Auto-imported)

### Basic Functions

```scheme
;; Identity
(id x) → x

;; Const
(const x y) → x

;; Compose
(comp f g) → (lambda [x] (f (g x)))

;; Flip
(flip f) → (lambda [x y] (f y x))
```

### Arithmetic

```scheme
(+ x y ...)   ;; Addition
(- x y ...)   ;; Subtraction
(* x y ...)   ;; Multiplication
(/ x y ...)   ;; Division
(mod x y)     ;; Modulo
(abs x)       ;; Absolute value
```

### Comparison

```scheme
(= x y)       ;; Equality
(!= x y)      ;; Inequality
(< x y)       ;; Less than
(<= x y)      ;; Less or equal
(> x y)       ;; Greater than
(>= x y)      ;; Greater or equal
```

### Logic

```scheme
(not x)       ;; Logical not
(and x y ...) ;; Short-circuit and
(or x y ...)  ;; Short-circuit or
```

## List Module

### Construction

```scheme
(list x y z)      ;; Create list
(cons x lst)      ;; Prepend element
(append lst1 lst2) ;; Concatenate lists
```

### Accessors

```scheme
(first lst)       ;; Head
(rest lst)        ;; Tail
(last lst)        ;; Last element
(nth lst n)       ;; Element at index
```

### Predicates

```scheme
(empty? lst)      ;; Check if empty
(member? x lst)   ;; Check membership
(any pred lst)    ;; Any element matches
(all pred lst)    ;; All elements match
```

### Transformations

```scheme
(map f lst)            ;; Apply function
(filter pred lst)      ;; Keep matching
(reject pred lst)      ;; Remove matching
(fold f init lst)      ;; Left fold
(fold-right f init lst) ;; Right fold
(reverse lst)          ;; Reverse list
(sort cmp lst)         ;; Sort by comparator
(unique lst)           ;; Remove duplicates
```

### Subsequences

```scheme
(take n lst)           ;; First n elements
(drop n lst)           ;; Skip n elements
(take-while pred lst)  ;; Take until false
(drop-while pred lst)  ;; Drop until false
(slice lst start end)  ;; Subsequence
```

## String Module

### Construction

```scheme
(string-concat s1 s2 ...) ;; Concatenate
(string-repeat s n)       ;; Repeat n times
(string-join lst sep)     ;; Join with separator
```

### Inspection

```scheme
(string-length s)         ;; Length
(string-get s n)          ;; Character at index
(string-contains? s sub)  ;; Check substring
(string-starts? s pre)    ;; Check prefix
(string-ends? s suf)      ;; Check suffix
```

### Transformation

```scheme
(string-trim s)           ;; Remove whitespace
(string-upper s)          ;; Uppercase
(string-lower s)          ;; Lowercase
(string-replace s old new) ;; Replace substring
(string-split s sep)      ;; Split by separator
```

## Math Module

### Constants

```scheme
math/pi       ;; 3.14159...
math/e        ;; 2.71828...
math/inf      ;; Infinity
math/nan      ;; Not-a-number
```

### Functions

```scheme
(sqrt x)      ;; Square root
(pow x y)     ;; Exponentiation
(exp x)       ;; e^x
(log x)       ;; Natural logarithm
(floor x)     ;; Round down
(ceil x)      ;; Round up
(round x)     ;; Round to nearest
(sin x)       ;; Sine
(cos x)       ;; Cosine
(tan x)       ;; Tangent
(min x y)     ;; Minimum
(max x y)     ;; Maximum
```

## Option Module

### Type

```scheme
(deftype Option
  (Some value)
  None)
```

### Operations

```scheme
(option/map f opt)         ;; Transform value
(option/flat-map f opt)    ;; Chain operations
(option/unwrap opt default) ;; Extract or default
(option/is-some? opt)      ;; Check if Some
(option/is-none? opt)      ;; Check if None
```

## Result Module

### Type

```scheme
(deftype Result
  (Ok value)
  (Err error))
```

### Operations

```scheme
(result/map f res)         ;; Transform Ok value
(result/map-err f res)     ;; Transform Err value
(result/flat-map f res)    ;; Chain operations
(result/unwrap res)        ;; Extract or panic
(result/unwrap-or res def) ;; Extract or default
(result/is-ok? res)        ;; Check if Ok
(result/is-err? res)       ;; Check if Err
```

## IO Module

### File Operations

```scheme
(io/read-file path)        : String -> String / {IO}
(io/write-file path data)  : String -> String -> Unit / {IO}
(io/append-file path data) : String -> String -> Unit / {IO}
(io/file-exists? path)     : String -> Bool / {IO}
(io/delete-file path)      : String -> Unit / {IO}
```

### Console

```scheme
(io/print x)               : a -> Unit / {IO}
(io/println x)             : a -> Unit / {IO}
(io/read-line)             : Unit -> String / {IO}
```

## Map Module

### Construction

```scheme
(map/empty)                ;; Empty map
(map/from-list lst)        ;; From key-value pairs
(map/set m k v)            ;; Add/update key
(map/remove m k)           ;; Remove key
```

### Access

```scheme
(map/get m k)              ;; Get value
(map/get-or m k default)   ;; Get with default
(map/has? m k)             ;; Check key exists
(map/keys m)               ;; All keys
(map/values m)             ;; All values
```

### Transformation

```scheme
(map/map f m)              ;; Transform values
(map/filter pred m)        ;; Keep matching pairs
(map/merge m1 m2)          ;; Merge maps
```

## Set Module

```scheme
(set/empty)                ;; Empty set
(set/from-list lst)        ;; From list
(set/add s x)              ;; Add element
(set/remove s x)           ;; Remove element
(set/member? s x)          ;; Check membership
(set/union s1 s2)          ;; Set union
(set/intersection s1 s2)   ;; Set intersection
(set/difference s1 s2)     ;; Set difference
(set/subset? s1 s2)        ;; Subset check
```

## Vector Module

```scheme
(vector/empty)             ;; Empty vector
(vector/from-list lst)     ;; From list
(vector/get v n)           ;; Element at index
(vector/set v n x)         ;; Update at index
(vector/push v x)          ;; Append element
(vector/pop v)             ;; Remove last
(vector/length v)          ;; Size
```

## Success Criteria

- [ ] Prelude functions implemented
- [ ] List operations complete
- [ ] String manipulation functions
- [ ] Math functions
- [ ] Option/Result types
- [ ] IO operations (with effect tracking)
- [ ] Map/Set/Vector data structures
- [ ] All standard library test files pass
- [ ] Documentation for all functions

## Test Coverage

Based on existing test files, the standard library already has substantial test coverage across 305 .omni test files.

## Next Phase

**Phase 5.2: Error Handling** will implement:
- Result types for recoverable errors
- Exception handling patterns
- Proper error messages

---

**Implementation Note:** Many standard library functions already exist in test files - Phase 5.1 organizes and documents them systematically.

---

**⚠️ HVM4 IMPLEMENTATION ONLY - DO NOT IMPLEMENT IN C INTERPRETER ⚠️**
