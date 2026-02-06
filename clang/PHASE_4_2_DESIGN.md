# Phase 4.2: Macro System

**Status:** Design Phase  
**Date:** 2026-02-06 (Updated)  
**Depends on:** Phase 4.1 (Module System)

## Overview

The macro system provides compile-time metaprogramming through:
- **Pattern-based** transformations using `define [syntax ...]`
- **Hygienic** expansion (prevents variable capture)
- **Quasiquote** for code generation
- **Reader macros** (syntax extensions)

**OmniLisp has ONE macro system**, not two. All macros use `define [syntax name]` with pattern matching.

## Motivation

**Without macros:**
```lisp
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
```lisp
;; Define once, use everywhere
(define [syntax assert-range]
  [(assert-range ?val ?min ?max ?msg)
   `(if (or (< ,?val ,?min) (> ,?val ,?max))
      (do
        (perform log ,?msg)
        (error ,?msg))
      ,?val)])

(sqrt (assert-range x 0 inf "negative value"))
(process (assert-range y 0 100 "too large"))
```

## Macro Definition Syntax

### Basic Pattern Matching

```lisp
(define [syntax when]
  [(when ?test ?body ...)
   (if ?test (do ?body ...) nothing)])

;; Usage
(when (> x 0)
  (print "positive")
  (increment-counter))

;; Expands to:
(if (> x 0)
  (do
    (print "positive")
    (increment-counter))
  nothing)
```

### Pattern Variables

| Syntax | Meaning |
|--------|---------|
| `?name` | Capture single form |
| `?name ...` | Capture zero or more forms |
| `literal` | Match exact symbol |

### Multiple Patterns

```lisp
(define [syntax my-and]
  [(my-and) true]
  [(my-and ?x) ?x]
  [(my-and ?x ?rest ...)
   (if ?x (my-and ?rest ...) false)])
```

## Code Generation with Quasiquote

### Quasiquote Basics

```lisp
`expr         ; Quote entire expression
,expr         ; Unquote (evaluate) expr
,@expr        ; Unquote-splice (spread list)
```

### Example: For Loop

```lisp
(define [syntax for]
  [(for [?var ?start ?end] ?body ...)
   (let ([loop-name (gensym "loop")])
     `(letrec ([,loop-name
                 (lambda [,?var]
                   (if (< ,?var ,?end)
                     (do ,@?body
                         (,loop-name (+ ,?var 1)))
                     nothing))])
        (,loop-name ,?start)))])

;; Usage
(for [i 0 10]
  (print i))
```

## Literal Keywords

Specify keywords that must match exactly:

```lisp
(define [syntax cond]
  [literals else]
  [(cond (else ?body ...)) (do ?body ...)]
  [(cond (?test ?result)) (if ?test ?result nothing)]
  [(cond (?test ?result) ?rest ...)
   (if ?test ?result (cond ?rest ...))])

;; Usage
(cond
  ((= x 0) "zero")
  ((> x 0) "positive")
  (else "negative"))
```

## Hygiene

### Problem: Variable Capture

**Unhygienic macro:**
```lisp
(define [syntax swap]
  [(swap ?a ?b)
   `(let [temp ,?a]
      (do (set! ,?a ,?b)
          (set! ,?b temp)))])

(let [temp 5] [x 10] [y 20]
  (swap x y)
  temp)  ; Returns 10, not 5! 'temp' was captured
```

**Hygienic solution:**
```lisp
(define [syntax swap]
  [(swap ?a ?b)
   (let ([temp-var (gensym "temp")])
     `(let [,temp-var ,?a]
        (do (set! ,?a ,?b)
            (set! ,?b ,temp-var))))])

(let [temp 5] [x 10] [y 20]
  (swap x y)
  temp)  ; Returns 5 - unique temp variable
```

### Gensym

Generate unique symbols:

```lisp
(gensym)         ; → g__1234
(gensym "loop")  ; → loop__5678
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

```lisp
(when (> x 0) (print x) (increment))

→ expand (when ...)
→ (if (> x 0) (do (print x) (increment)) nothing)
→ expand (if ...)
→ (if (> x 0) (do (print x) (increment)) nothing)  # Primitive, done
```

## Standard Macros

### Control Flow

```lisp
;; when - conditional with implicit do
(define [syntax when]
  [(when ?test ?body ...)
   (if ?test (do ?body ...) nothing)])

;; unless - inverted conditional
(define [syntax unless]
  [(unless ?test ?body ...)
   (if ?test nothing (do ?body ...))])

;; cond - multi-way conditional
(define [syntax cond]
  [literals else]
  [(cond (else ?body ...)) (do ?body ...)]
  [(cond (?test ?body ...))
   (if ?test (do ?body ...) nothing)]
  [(cond (?test ?body ...) ?rest ...)
   (if ?test (do ?body ...) (cond ?rest ...))])
```

### Binding

```lisp
;; let* - sequential let
(define [syntax let*]
  [(let* () ?body ...) (do ?body ...)]
  [(let* ([?name ?val] ?rest ...) ?body ...)
   (let [?name ?val]
     (let* (?rest ...) ?body ...))])

;; and - short-circuit conjunction
(define [syntax and]
  [(and) true]
  [(and ?test) ?test]
  [(and ?test ?rest ...)
   (if ?test (and ?rest ...) false)])

;; or - short-circuit disjunction
(define [syntax or]
  [(or) false]
  [(or ?test) ?test]
  [(or ?test ?rest ...)
   (let [temp ?test]
     (if temp temp (or ?rest ...)))])
```

### Loops

```lisp
;; while - imperative loop
(define [syntax while]
  [(while ?test ?body ...)
   (let ([loop (gensym "while")])
     `(letrec ([,loop (lambda []
                        (if ,?test
                          (do ,@?body (,loop))
                          nothing))])
        (,loop)))])

;; for-each - iterate over list
(define [syntax for-each]
  [(for-each [?var ?lst] ?body ...)
   `(letrec ([loop (lambda [remaining]
                     (match remaining
                       () nothing
                       (h .. t) (do
                                 (let [,?var h]
                                   ,@?body)
                                 (loop t))))])
      (loop ,?lst))])
```

## Advanced Features

### Nested Ellipsis

```lisp
(define [syntax matrix]
  [(matrix [[?val ...] ...])
   `[,@(list ,@?val ...) ...]])

;; Usage
(matrix [[1 2 3] [4 5 6]])
;; → [[1 2 3] [4 5 6]]
```

### Anaphoric Macros

```lisp
(define [syntax aif]  ; anaphoric if
  [(aif ?test ?then ?else)
   `(let [it ,?test]
      (if it ,?then ,?else))])

;; Usage
(aif (find 42 my-list)
  (print it)  ; 'it' bound to result
  (print "not found"))
```

## Reader Macros

Syntax extensions at read time:

```lisp
;; Define #[...] as set literal
(define-reader-macro "["
  (lambda [reader]
    (let ([items (read-delimited reader "]")])
      `(set ,@items))))

;; Usage
#[1 2 3 4]  ; → (set 1 2 3 4)
```

## Module Integration

### Macro Export

```lisp
(module mylib
  (export when unless defstruct)
  
  (define [syntax when] ...)
  (define [syntax unless] ...)
  (define [syntax defstruct] ...))

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
typedef struct SyntaxRule {
    Term *pattern;        // e.g., (when ?test ?body ...)
    Term *template;       // e.g., (if ?test (do ?body ...) nothing)
    char **literals;      // Literal identifiers (e.g., "else")
    size_t literal_count;
} SyntaxRule;

typedef struct Macro {
    char *name;           // e.g., "when"
    SyntaxRule *rules;    // Pattern-template pairs
    size_t rule_count;
} Macro;
```

### Phase 4.2.2: Pattern Matching

```c
typedef struct PatternMatch {
    char **var_names;     // e.g., ["?test", "?body"]
    Term **var_values;    // Captured values
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

## Examples

### Example 1: Assert Macro

```lisp
(define [syntax assert]
  [(assert ?condition ?message)
   `(if (not ,?condition)
      (error (format "Assertion failed: ~a" ,?message))
      nothing)])

(assert (> x 0) "x must be positive")
```

### Example 2: With-Resource Macro

```lisp
(define [syntax with-resource]
  [(with-resource [?var ?init] ?body ...)
   (let ([cleanup (gensym "cleanup")])
     `(let [,?var ,?init]
            [,cleanup (lambda [] (close ,?var))]
        (try
          (do ,@?body)
          (finally (,cleanup)))))])

(with-resource [file (open "data.txt")]
  (read-lines file))
```

### Example 3: Memoization Macro

```lisp
(define [syntax defmemo]
  [(defmemo ?name [?arg] ?body)
   (let ([cache (gensym "cache")])
     `(do
        (define ,cache (make-hash))
        (define ,?name [,?arg]
          (if (hash-has? ,cache ,?arg)
            (hash-get ,cache ,?arg)
            (let [result ,?body]
              (hash-set! ,cache ,?arg result)
              result)))))])

(defmemo fib [n]
  (if (<= n 1) n
    (+ (fib (- n 1)) (fib (- n 2)))))
```

## Success Criteria

- [ ] `define [syntax ...]` pattern matching
- [ ] Pattern variables with `?name` and `?name ...`
- [ ] Literal keyword matching
- [ ] Quasiquote code generation
- [ ] Hygienic expansion with gensym
- [ ] Recursive macro expansion
- [ ] Ellipsis patterns (`...`)
- [ ] Macro export/import
- [ ] bound-identifier=? and free-identifier=?
- [ ] Reader macros
- [ ] All macro test files pass

## Next Phase

**Phase 4.3: Pattern Matching** will implement:
- Pattern destructuring in function arguments
- Match expressions with guards
- Exhaustiveness checking

---

**Implementation Approach:** Full hygienic macro system for HVM4.

---

**⚠️ HVM4 IMPLEMENTATION ONLY - DO NOT IMPLEMENT IN C INTERPRETER ⚠️**
