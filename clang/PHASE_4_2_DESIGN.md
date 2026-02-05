# Phase 4.2: Macro System

**Status:** Design Phase  
**Date:** 2026-02-05  
**Depends on:** Phase 4.1 (Module System)

## Overview

The macro system provides compile-time metaprogramming through:
- **Pattern-based** transformations (syntax-rules)
- **Procedural** macros (define-syntax with full code)
- **Hygienic** expansion (prevents variable capture)
- **Reader macros** (syntax extensions)

## Motivation

**Without macros:**
```scheme
;; Repetitive code
(if (< x 0)
  (do
    (perform log "negative")
    (error "negative value"))
  (sqrt x))

(if (> y 100)
  (do
    (perform log "too large")
    (error "too large"))
  (process y))
```

**With macros:**
```scheme
;; Define once, use everywhere
(define-syntax assert-range
  [(_ val min max msg)
   `(if (or (< ,val ,min) (> ,val ,max))
      (do
        (perform log ,msg)
        (error ,msg))
      ,val)])

(sqrt (assert-range x 0 inf "negative value"))
(process (assert-range y 0 100 "too large"))
```

## Macro Types

### 1. Pattern-Based Macros (syntax-rules)

Simple pattern matching and substitution:

```scheme
(define-syntax when
  (syntax-rules ()
    [(_ test body ...)
     (if test (do body ...) unit)]))

;; Usage
(when (> x 0)
  (print "positive")
  (increment-counter))

;; Expands to:
(if (> x 0)
  (do
    (print "positive")
    (increment-counter))
  unit)
```

### 2. Procedural Macros (define-syntax)

Full code generation with quasiquote:

```scheme
(define-syntax for
  [(_ [var start end] body ...)
   (let ([loop-name (gensym "loop")])
     `(letrec ([,loop-name
                 (lambda [,var]
                   (if (< ,var ,end)
                     (do ,@body
                         (,loop-name (+ ,var 1)))
                     unit))])
        (,loop-name ,start)))])

;; Usage
(for [i 0 10]
  (print i))
```

### 3. Reader Macros

Syntax extensions at read time:

```scheme
;; Define #[...] as set literal
(define-reader-macro "[" 
  (lambda [reader]
    (let ([items (read-delimited reader "]")])
      `(set ,@items))))

;; Usage
#[1 2 3 4]  ;; → (set 1 2 3 4)
```

## Hygiene

### Problem: Variable Capture

**Unhygienic macro:**
```scheme
(define-syntax swap
  [(_ a b)
   `(let ([temp ,a])
      (set! ,a ,b)
      (set! ,b temp))])

(let ([temp 5] [x 10] [y 20])
  (swap x y)
  temp)  ;; Returns 10, not 5! 'temp' was captured
```

**Hygienic solution:**
```scheme
(define-syntax swap
  [(_ a b)
   (let ([temp-var (gensym "temp")])
     `(let ([,temp-var ,a])
        (set! ,a ,b)
        (set! ,b ,temp-var)))])

(let ([temp 5] [x 10] [y 20])
  (swap x y)
  temp)  ;; Returns 5 - unique temp variable
```

### Gensym

Generate unique symbols:

```scheme
(gensym)         ;; → g__1234
(gensym "loop")  ;; → loop__5678
```

### Identifier Comparison

```scheme
;; bound-identifier=?
;; Compares binding identity
(bound-identifier=? x x)      ;; → true
(bound-identifier=? x y)      ;; → false

;; free-identifier=?
;; Compares symbolic names
(free-identifier=? 'map 'map)  ;; → true
```

## Macro Expansion

### Expansion Phases

```
1. Read      - Parse source into AST
2. Expand    - Apply macros recursively
3. Analyze   - Type/effect inference
4. Compile   - Generate code
```

### Expansion Algorithm

```
expand(expr):
  if expr is macro call:
    result = invoke_macro(expr)
    return expand(result)  # Recursive
  else if expr is list:
    return map(expand, expr)
  else:
    return expr
```

### Example Trace

```scheme
(when (> x 0) (print x) (increment))

→ expand (when ...)
→ (if (> x 0) (do (print x) (increment)) unit)
→ expand (if ...)
→ (if (> x 0) (do (print x) (increment)) unit)  # Primitive, done
```

## Standard Macros

### Control Flow

```scheme
;; when - conditional with implicit begin
(define-syntax when
  (syntax-rules ()
    [(_ test body ...) 
     (if test (do body ...) unit)]))

;; unless - inverted conditional
(define-syntax unless
  (syntax-rules ()
    [(_ test body ...) 
     (if test unit (do body ...))]))

;; cond - multi-way conditional
(define-syntax cond
  (syntax-rules (else)
    [(_ [else body ...]) (do body ...)]
    [(_ [test body ...]) (if test (do body ...) unit)]
    [(_ [test body ...] rest ...)
     (if test (do body ...) (cond rest ...))]))
```

### Binding

```scheme
;; let* - sequential let
(define-syntax let*
  (syntax-rules ()
    [(_ () body ...) (do body ...)]
    [(_ ([name val] rest ...) body ...)
     (let [name val]
       (let* (rest ...) body ...))]))

;; and - short-circuit conjunction
(define-syntax and
  (syntax-rules ()
    [(_) true]
    [(_ test) test]
    [(_ test rest ...)
     (if test (and rest ...) false)]))
```

### Loops

```scheme
;; while - imperative loop
(define-syntax while
  [(_ test body ...)
   (let ([loop (gensym "while")])
     `(letrec ([,loop (lambda []
                        (if ,test
                          (do ,@body (,loop))
                          unit))])
        (,loop)))])

;; for-each - iterate over list
(define-syntax for-each
  [(_ [var lst] body ...)
   `(letrec ([loop (lambda [remaining]
                     (match remaining
                       () unit
                       (h .. t) (do
                                 (let [,var h]
                                   ,@body)
                                 (loop t))))])
      (loop ,lst))])
```

## Advanced Features

### Syntax Parameters

Establish compile-time parameters:

```scheme
(define-syntax-parameter it
  (lambda [stx] (error "it not in context")))

(define-syntax aif  ; anaphoric if
  [(_ test then else)
   `(let [result ,test]
      (syntax-parameterize ([it (lambda [stx] 'result)])
        (if result ,then ,else)))])

;; Usage
(aif (find 42 my-list)
  (print it)  ; 'it' refers to result
  (print "not found"))
```

### Syntax Classes

Pattern matching with constraints:

```scheme
(define-syntax-class identifier
  #:description "an identifier"
  (pattern x:id))

(define-syntax-class expr
  #:description "an expression"
  (pattern e))

(define-syntax my-let
  [(_ ([name:identifier val:expr] ...) body:expr ...)
   `(let ,(map list names vals) ,@body)])
```

### Ellipsis Patterns

Repetition in patterns:

```scheme
(define-syntax let
  (syntax-rules ()
    [(_ ([name val] ...) body ...)
     ((lambda [name ...] body ...) val ...)]))

;; ... means "zero or more"
```

## Module Integration

### Macro Export

```scheme
(module mylib
  (export when unless defstruct)
  
  (define-syntax when ...)
  (define-syntax unless ...)
  (define-syntax defstruct ...))

;; Import macros
(import mylib)
(when (> x 0) (print x))  ; Macro available
```

### Macro Expansion Order

```
1. Module-level macros expand first
2. Top-level defines processed
3. Function bodies expanded last
```

## Implementation Strategy

### Phase 4.2.1: Macro Representation

```c
typedef enum {
    MACRO_SYNTAX_RULES,   // Pattern-based
    MACRO_PROCEDURAL,     // Full code
    MACRO_READER,         // Read-time
} MacroKind;

typedef struct SyntaxRule {
    Term *pattern;
    Term *template;
    char **literals;    // Literal identifiers
    size_t literal_count;
} SyntaxRule;

typedef struct Macro {
    char *name;
    MacroKind kind;
    union {
        struct {
            SyntaxRule *rules;
            size_t rule_count;
        } syntax_rules;
        Term *procedure;    // Transformer function
    } data;
} Macro;
```

### Phase 4.2.2: Pattern Matching

```c
typedef struct PatternMatch {
    char **var_names;
    Term **var_values;
    size_t count;
} PatternMatch;

// Match pattern against term
PatternMatch *match_pattern(Term *pattern, 
                           Term *term,
                           char **literals);

// Check if match succeeded
bool is_match_success(PatternMatch *m);
```

### Phase 4.2.3: Template Instantiation

```c
// Instantiate template with matched bindings
Term *instantiate_template(Term *template,
                          PatternMatch *bindings);

// Handle ellipsis expansion
Term *expand_ellipsis(Term *template,
                     PatternMatch *bindings,
                     char *ellipsis_var);
```

### Phase 4.2.4: Macro Expander

```c
typedef struct MacroRegistry {
    Macro **macros;
    size_t count;
} MacroRegistry;

// Register macro
void register_macro(MacroRegistry *reg, Macro *macro);

// Expand macro call
Term *expand_macro(Term *call, 
                  MacroRegistry *reg);

// Recursive expansion
Term *expand_all(Term *expr, 
                MacroRegistry *reg);
```

### Phase 4.2.5: Hygiene Implementation

```c
typedef struct SyntaxContext {
    int *marks;         // Hygiene marks
    size_t mark_count;
    Module *module;     // Lexical scope
} SyntaxContext;

// Generate unique symbol
Term *gensym(const char *prefix);

// Add hygiene mark
Term *add_mark(Term *term, int mark);

// Compare identifiers
bool bound_identifier_eq(Term *id1, Term *id2);
bool free_identifier_eq(Term *id1, Term *id2);
```

## Test Coverage

Test files validating the macro system:

- `test_define_syntax.omni` - Basic macro definition
- `test_macro_advanced.omni` - Complex patterns
- `test_macroexpand.omni` - Expansion mechanics
- `test_macroexpand_all.omni` - Recursive expansion
- `test_syntax_hygiene.omni` - Hygienic expansion
- `test_syntax_pattern.omni` - Pattern matching
- `test_syntax_ellipsis.omni` - Ellipsis patterns
- `test_syntax_literals.omni` - Literal matching
- `test_reader_macros.omni` - Reader extensions

## Examples

### Example 1: Assert Macro

```scheme
(define-syntax assert
  [(_ condition message)
   `(if (not ,condition)
      (error (format "Assertion failed: ~a" ,message))
      unit)])

(assert (> x 0) "x must be positive")
```

### Example 2: With-Resource Macro

```scheme
(define-syntax with-resource
  [(_ [var init] body ...)
   (let ([cleanup (gensym "cleanup")])
     `(let ([,var ,init]
            [,cleanup (lambda [] (close ,var))])
        (try
          (do ,@body)
          (finally (,cleanup)))))])

(with-resource [file (open "data.txt")]
  (read-lines file))
```

### Example 3: Memoization Macro

```scheme
(define-syntax defmemo
  [(_ name [arg] body)
   (let ([cache (gensym "cache")])
     `(do
        (define ,cache (make-hash))
        (define ,name [,arg]
          (if (hash-has? ,cache ,arg)
            (hash-get ,cache ,arg)
            (let [result ,body]
              (hash-set! ,cache ,arg result)
              result)))))])

(defmemo fib [n]
  (if (<= n 1) n
    (+ (fib (- n 1)) (fib (- n 2)))))
```

## Success Criteria

- [ ] syntax-rules pattern matching
- [ ] Procedural macros with quasiquote
- [ ] Hygienic expansion with gensym
- [ ] Recursive macro expansion
- [ ] Ellipsis patterns
- [ ] Macro export/import
- [ ] bound-identifier=? and free-identifier=?
- [ ] Syntax parameters
- [ ] Reader macros
- [ ] All macro test files pass

## Next Phase

**Phase 4.3: Pattern Matching** will implement:
- Pattern destructuring in function arguments
- Match expressions with guards
- Exhaustiveness checking

---

**Implementation Approach:** Full hygienic macro system for HVM. C interpreter could support basic syntax-rules.

---

**⚠️ HVM4 IMPLEMENTATION ONLY - DO NOT IMPLEMENT IN C INTERPRETER ⚠️**
