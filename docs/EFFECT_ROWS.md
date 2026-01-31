# Effect Rows in OmniLisp

This document explains OmniLisp's effect row system, which tracks what effects a function may perform.

## Overview

Effect rows are type-level annotations that describe which effects a function may perform. This enables:

1. **Automatic parallelization**: Effect-free functions can be safely parallelized
2. **Static analysis**: Detect when effects escape their handlers
3. **Documentation**: Explicit effect signatures clarify function behavior

## Syntax

### Annotating Effect Rows

Use `^:effects [...]` to annotate a function's effects:

```lisp
;; Function that performs IO effects
(define log-value [x]
  ^:effects [IO]
  (do (println x) x))

;; Function that performs State and IO
(define update-and-log [x]
  ^:effects [State IO]
  (do (set! counter (+ counter 1))
      (println x)
      x))

;; Effect-free function (empty effect row)
(define add [x {Int}] [y {Int}]
  ^:effects []
  (+ x y))
```

### The `^:pure` Shorthand

`^:pure` is equivalent to `^:effects []`:

```lisp
;; These are equivalent:
(define add ^:pure [x] [y] (+ x y))
(define add ^:effects [] [x] [y] (+ x y))
```

## Effect Inference

For functions without explicit effect annotations, OmniLisp performs **conservative effect inference**.

### How Inference Works

The inference algorithm walks the function body AST and looks for:

| Operation | Effect |
|-----------|--------|
| `(perform effect payload)` | Has effect `effect` |
| `(println ...)` | Has IO effect |
| `(print ...)` | Has IO effect |
| `(set! ...)` | Has State effect |
| `(put! ...)` | Has State effect |
| FFI calls | Assumed effectful |

### Pure Operations

These operations are always effect-free:

- Arithmetic: `+`, `-`, `*`, `/`, `mod`
- Comparison: `=`, `!=`, `<`, `>`, `<=`, `>=`
- Data construction: lists, arrays, records
- Pattern matching: `match`, `if`
- Let bindings: `let`, `let*`
- Lambda creation
- Handled effect blocks (effects are encapsulated)

### Example

```lisp
;; Effect inference determines this is pure
(define double [x] (* x 2))
;; (effect-free? double) => #True{}

;; Effect inference determines this has effects
(define log-double [x]
  (do (println x) (* x 2)))
;; (effect-free? log-double) => #Fals{}
```

## Using Effect Information

### The `effect-free?` Predicate

Check if a function is effect-free:

```lisp
(effect-free? double)      ;; => #True{}
(effect-free? log-double)  ;; => #Fals{}
```

### Smart Dispatch

The standard library uses effect information for automatic optimization:

```lisp
;; smart_map checks effect-free? and chooses implementation
(smart_map f xs)
;; If f is effect-free: parallel map
;; If f has effects: sequential map

;; In prelude.hvm4:
@smart_map = λf. λxs.
  λ{
    #True: @map(f)(xs)       // parallel
    _:     @map_seq(f)(xs)   // sequential
  }(#Effr{f})
```

## Internal Representation

### AST Nodes

| Node | Description |
|------|-------------|
| `#ERws{effects}` | Effect row containing list of effect types |
| `#Effr{func}` | Effect-free check node |
| `#Pure{func}` | Purity wrapper (marks function as `^:pure`) |

### Method Structure

Methods store their effect row in the 5th field:

```
#Meth{name, signature, implementation, constraints, effects}
                                                    ^^^^^^^
                                                    #ERws{list} or #NIL
```

## Effect Row Polymorphism (Future)

Planned support for effect row variables:

```lisp
;; E is an effect row variable
(define map [f {(-> A B) ^:effects E}] [xs {(List A)}]
  ^:effects E
  ...)

;; The effect row of map matches whatever f performs
```

## Best Practices

1. **Mark pure functions explicitly** with `^:pure` when the inference might be conservative
2. **Use `handle` blocks** to encapsulate effects and make composed operations pure
3. **Check with `effect-free?`** during development to verify purity assumptions
4. **Prefer pure functions** for better parallelization opportunities

## Related Documentation

- [EFFECTS_AND_HANDLERS.md](./EFFECTS_AND_HANDLERS.md) - Algebraic effects system
- [ADVANCED_FEATURES.md](./ADVANCED_FEATURES.md) - Fibers and concurrency
- [QUICK_REFERENCE.md](./QUICK_REFERENCE.md) - Language overview
