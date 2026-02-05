# Phase 4.3: Pattern Matching

**Status:** Design Phase  
**Date:** 2026-02-05  
**Depends on:** Phase 4.2 (Macro System)

## Overview

Pattern matching provides declarative destructuring of data structures with:
- **Structural** patterns (lists, arrays, records)
- **Guard** clauses (conditional matching)
- **Or** patterns (multiple patterns for one clause)
- **As** patterns (bind while matching subpattern)
- **Exhaustiveness** checking (compile-time completeness)

## Motivation

**Without pattern matching:**
```scheme
(define process-list [lst]
  (if (empty? lst)
    0
    (let ([h (first lst)]
          [t (rest lst)])
      (if (pair? h)
        (let ([a (first h)]
              [b (second h)])
          (+ a (+ b (process-list t))))
        (+ h (process-list t))))))
```

**With pattern matching:**
```scheme
(define process-list [lst]
  (match lst
    ()              0
    ((a b) .. rest) (+ a (+ b (process-list rest)))
    (h .. rest)     (+ h (process-list rest))))
```

## Pattern Syntax

### Basic Patterns

```scheme
42              ;; Literal number
"hello"         ;; Literal string
true            ;; Literal boolean
x               ;; Variable binding
_               ;; Wildcard (matches anything, no binding)
```

### List Patterns

```scheme
()              ;; Empty list
(x)             ;; Single-element list
(x y)           ;; Two-element list
(x .. rest)     ;; Head + tail
(x y .. rest)   ;; Two elements + tail
```

### Array/Vector Patterns

```scheme
#[]             ;; Empty array
#[x]            ;; Single element
#[x y z]        ;; Fixed size
#[x ... rest]   ;; Variable length
```

### Record/Map Patterns

```scheme
{:x x :y y}     ;; Match and bind fields
{:x a}          ;; Match :x, bind to 'a'
{:x 42}         ;; Match field with literal value
```

### Or Patterns

```scheme
(0 | 1)         ;; Match 0 or 1
("yes" | "y")   ;; Match either string
```

### As Patterns

```scheme
pair@(x y)      ;; Bind pair and elements
lst@(h .. t)    ;; Bind whole list and parts
```

### Guard Patterns

```scheme
x & (> x 0)     ;; Match and check condition
n & (even? n)   ;; Match even numbers
```

## Match Expression

### Basic Match

```scheme
(match expr
  pattern1  result1
  pattern2  result2
  ...
  _         default)
```

### Example: List Processing

```scheme
(define length [lst]
  (match lst
    ()        0
    (_ .. t)  (+ 1 (length t))))
```

### Example: Tree Traversal

```scheme
(deftype Tree
  (Leaf value)
  (Node left value right))

(define sum-tree [tree]
  (match tree
    (Leaf n)           n
    (Node left v right) (+ v (+ (sum-tree left) 
                                (sum-tree right)))))
```

## Guard Patterns

### Simple Guards

```scheme
(define classify [n]
  (match n
    x & (< x 0)   "negative"
    0             "zero"
    x & (> x 0)   "positive"))
```

### Complex Guards

```scheme
(define process [item]
  (match item
    (x y) & (= x y)        "equal pair"
    (x y) & (> x y)        "decreasing"
    (x y) & (< x y)        "increasing"
    _                      "other"))
```

### Guards with Multiple Bindings

```scheme
(match point
  {:x x :y y} & (and (> x 0) (> y 0))  "quadrant-1"
  {:x x :y y} & (and (< x 0) (> y 0))  "quadrant-2"
  _                                     "other")
```

## Nested Patterns

### Nested Lists

```scheme
(match expr
  ((a b) c)          ;; List of pair and element
  (a (b c))          ;; Element and pair
  (((a .. _) .. _) .. _)  ;; Deeply nested
```

### Mixed Structures

```scheme
(match data
  {:user {:name n :age a}}  ;; Nested records
  #[{:x x} {:y y}]          ;; Array of records
  (({:id i} val) .. rest)   ;; List of pairs
```

## Or Patterns

### Alternatives

```scheme
(match value
  (0 | 1)          "binary"
  (2 | 3 | 5 | 7)  "small-prime"
  _                "other")
```

### With Bindings

```scheme
(match result
  (Ok val | Some val)  val
  (Err _ | None)       default-value)
```

## As Patterns

### Bind Whole and Parts

```scheme
(match lst
  pair@(a b)  (list pair a b)
  _           nothing)

;; pair = (1 2), a = 1, b = 2
```

### Nested As Patterns

```scheme
(match tree
  node@(Node left@(Leaf x) right)
    ;; node, left, x, right all bound
```

## Pattern Matching in Functions

### Destructuring Arguments

```scheme
;; Traditional
(define add [point]
  (let [x (:x point)]
    (let [y (:y point)]
      (+ x y))))

;; With pattern matching
(define add [{:x x :y y}]
  (+ x y))
```

### Multiple Clauses

```scheme
(define factorial
  [0]           1
  [n] & (> n 0) (* n (factorial (- n 1))))
```

## Exhaustiveness Checking

### Complete Patterns

```scheme
(match value
  0   "zero"
  _   "non-zero")  ;; Complete - wildcard catches all
```

### Incomplete (Warning)

```scheme
(match color
  Red   "red"
  Blue  "blue")
  ;; WARNING: Green not covered
```

### ADT Exhaustiveness

```scheme
(deftype Option
  Some
  None)

(match opt
  Some x  x
  None    default)  ;; Complete - all constructors covered
```

## Implementation Strategy

### Phase 4.3.1: Pattern Representation

```c
typedef enum {
    PATTERN_LITERAL,   // 42, "hello", true
    PATTERN_VAR,       // x
    PATTERN_WILDCARD,  // _
    PATTERN_LIST,      // (x y .. rest)
    PATTERN_ARRAY,     // #[x y z]
    PATTERN_RECORD,    // {:x x :y y}
    PATTERN_OR,        // (a | b)
    PATTERN_AS,        // name@pattern
    PATTERN_GUARD,     // pattern & condition
} PatternKind;

typedef struct Pattern {
    PatternKind kind;
    union {
        Term *literal;
        char *var_name;
        struct {
            Pattern **elements;
            size_t count;
            char *rest_var;  // For .. patterns
        } list;
        struct {
            Pattern *pattern;
            char *name;
        } as_pattern;
        struct {
            Pattern *pattern;
            Term *condition;
        } guard;
    } data;
} Pattern;
```

### Phase 4.3.2: Pattern Compilation

```c
typedef struct CompiledPattern {
    // Decision tree for efficient matching
    enum {
        TEST_TYPE,     // Check value type
        TEST_LITERAL,  // Check literal equality
        TEST_LENGTH,   // Check list/array length
        TEST_FIELD,    // Check record field
        BIND_VAR,      // Bind variable
        GUARD_CHECK,   // Evaluate guard
        SUCCESS,       // Match succeeded
        FAIL          // Match failed
    } node_kind;
    
    CompiledPattern *on_success;
    CompiledPattern *on_failure;
} CompiledPattern;

// Compile pattern to decision tree
CompiledPattern *compile_pattern(Pattern *pattern);
```

### Phase 4.3.3: Match Execution

```c
typedef struct MatchResult {
    bool success;
    Environment *bindings;  // Variable bindings
} MatchResult;

// Execute compiled pattern
MatchResult *execute_match(CompiledPattern *pattern,
                          Term *value);

// Try multiple patterns in sequence
MatchResult *try_patterns(CompiledPattern **patterns,
                         size_t count,
                         Term *value);
```

### Phase 4.3.4: Exhaustiveness Checker

```c
typedef struct CoverageMatrix {
    Pattern **patterns;
    bool *covered;
    size_t row_count;
} CoverageMatrix;

// Check pattern exhaustiveness
bool check_exhaustive(Pattern **patterns,
                     size_t count,
                     Type *value_type);

// Find uncovered cases
Pattern **find_missing_patterns(Pattern **patterns,
                               size_t count,
                               Type *value_type);
```

## Test Coverage

- `test_list_patterns.omni` - List destructuring
- `test_pattern_array.omni` - Array patterns
- `test_pattern_as.omni` - As patterns
- `test_pattern_binding.omni` - Variable binding
- `test_pattern_literals.omni` - Literal matching
- `test_pattern_nested.omni` - Nested patterns
- `test_pattern_or.omni` - Or patterns
- `test_pattern_wildcard.omni` - Wildcard patterns
- `test_guards.omni` - Guard patterns
- `test_guard_pattern.omni` - Complex guards
- `test_spread_pattern.omni` - Rest patterns

## Success Criteria

- [ ] Basic pattern syntax
- [ ] List/array/record patterns
- [ ] Nested patterns
- [ ] Or patterns
- [ ] As patterns
- [ ] Guard patterns
- [ ] Pattern compilation to decision trees
- [ ] Exhaustiveness checking
- [ ] Pattern matching in function definitions
- [ ] All pattern test files pass

## Next Phase

**Phase 5.1: Standard Library** will implement:
- Complete prelude functions
- Collection operations
- String/number utilities

---

**Implementation Approach:** Full pattern compilation for HVM. C interpreter uses runtime pattern matching.

---

**⚠️ HVM4 IMPLEMENTATION ONLY - DO NOT IMPLEMENT IN C INTERPRETER ⚠️**
