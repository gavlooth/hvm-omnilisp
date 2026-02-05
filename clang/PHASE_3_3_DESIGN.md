# Phase 3.3: Effect Inference

**Status:** Design Phase  
**Date:** 2026-02-05  
**Depends on:** Phase 3.1 (Effect Row Types), Phase 3.2 (Effect Handlers)

## Overview

Effect inference automatically determines which effects a computation may perform, without requiring explicit effect annotations. This enables:
- Concise code (no manual effect signatures)
- Automatic effect row construction
- Type-and-effect inference in one pass
- Polymorphic effect variables

## Motivation

**Without effect inference:**
```scheme
;; Manual effect annotation
(define fetch-and-log [url] : String -> Data / {Http, Log}
  (do
    (perform log "fetching...")
    (let [result (perform http-get url)]
      (perform log "done")
      result)))
```

**With effect inference:**
```scheme
;; Effects inferred automatically
(define fetch-and-log [url]
  (do
    (perform log "fetching...")
    (let [result (perform http-get url)]
      (perform log "done")
      result)))

;; Inferred type: String -> Data / {Http, Log}
```

## Inference Algorithm

### Phase 1: Constraint Generation

Walk the AST and generate effect constraints:

```
Infer(e, Γ) → (τ, ε, C)

where:
  τ = inferred type
  ε = effect row (may contain variables)
  C = set of constraints
```

**Rules:**

**Literals:**
```
Infer(42, Γ) → (Int, {}, ∅)
Infer("hello", Γ) → (String, {}, ∅)
```

**Variables:**
```
x : τ / ε ∈ Γ
────────────────
Infer(x, Γ) → (τ, ε, ∅)
```

**Perform:**
```
Infer((perform E p), Γ) → (α, {E}, ∅)

where α is fresh type variable
```

**Application:**
```
Infer(f, Γ) → (τ₁ -> τ₂ / ε₁, ε_f, C₁)
Infer(x, Γ) → (τ₃, ε_x, C₂)
────────────────────────────────────
Infer((f x), Γ) → (τ₂, ε_f ∪ ε_x ∪ ε₁, 
                    C₁ ∪ C₂ ∪ {τ₁ = τ₃})
```

**Lambda:**
```
Infer(body, Γ[x:α]) → (τ, ε, C)
─────────────────────────────────
Infer((lambda [x] body), Γ) → 
  ((α -> τ / ε), {}, C)
```

**Let:**
```
Infer(e₁, Γ) → (τ₁, ε₁, C₁)
Infer(e₂, Γ[x:τ₁]) → (τ₂, ε₂, C₂)
────────────────────────────────────
Infer((let [x e₁] e₂), Γ) → 
  (τ₂, ε₁ ∪ ε₂, C₁ ∪ C₂)
```

**Handle:**
```
Infer(body, Γ) → (τ, ε_body, C)
handlers handle effects E₁...Eₙ
─────────────────────────────────
Infer((handle body ...handlers), Γ) → 
  (τ, ε_body \ {E₁...Eₙ}, C)
```

### Phase 2: Constraint Solving

Solve collected constraints to determine:
1. Type unification
2. Effect row unification
3. Effect variable instantiation

```
solve(C) → σ

where σ is a substitution mapping type/effect 
variables to concrete types/effect rows
```

### Phase 3: Effect Row Simplification

Minimize effect rows:

```
simplify({State, Log, State, Http}) 
  → {State, Log, Http}

simplify(ε₁ ∪ ε₂) where ε₁ ⊆ ε₂
  → ε₂

simplify({E} ∪ (α \ {E}))
  → α
```

## Effect Polymorphism

Effect inference supports polymorphic effect variables:

```scheme
;; map is effect-polymorphic
(define map [f] [lst]
  (match lst
    () ()
    (h .. t) (cons (f h) (map f t))))

;; Inferred: (a -> b / ε) -> List a -> List b / ε
```

Usage examples:

```scheme
;; Pure mapping
(map (lambda [x] (* x 2)) '(1 2 3))
;; Type: List Int / {}

;; Effectful mapping
(map (lambda [x] (perform log x) x) '(1 2 3))
;; Type: List Int / {Log}
```

## Advanced Inference Patterns

### Pattern 1: Effect Abstraction

```scheme
(define with-state [init] [f]
  (handle-state init
    (f ())))

;; Inferred:
;; s -> (() -> a / {State} ∪ ε) -> a / ε
```

### Pattern 2: Effect Forwarding

```scheme
(define logged [f] [x]
  (do
    (perform log "before")
    (let [result (f x)]
      (perform log "after")
      result)))

;; Inferred:
;; (a -> b / ε) -> a -> b / ε ∪ {Log}
```

### Pattern 3: Conditional Effects

```scheme
(define maybe-log [should-log] [msg]
  (if should-log
    (perform log msg)
    unit))

;; Inferred:
;; Bool -> String -> Unit / {Log}
;; Note: Effect happens on one branch, so it's in the union
```

### Pattern 4: Effect Accumulation

```scheme
(define process-all [items]
  (foldl 
    (lambda [acc] [item]
      (do
        (perform log item)
        (perform state-update item)
        (+ acc 1)))
    0
    items))

;; Inferred:
;; List a -> Int / {Log, StateUpdate}
```

## Implementation Strategy

### Phase 3.3.1: Constraint System

```c
typedef enum {
    CONSTRAINT_TYPE_EQ,      // τ₁ = τ₂
    CONSTRAINT_EFFECT_EQ,    // ε₁ = ε₂
    CONSTRAINT_EFFECT_SUB,   // ε₁ ⊆ ε₂
    CONSTRAINT_EFFECT_UNION, // ε = ε₁ ∪ ε₂
} ConstraintKind;

typedef struct Constraint {
    ConstraintKind kind;
    union {
        struct { Type *t1; Type *t2; } type_eq;
        struct { EffectRow *e1; EffectRow *e2; } effect_eq;
        struct { EffectRow *sub; EffectRow *super; } effect_sub;
        struct { EffectRow *result; EffectRow *e1; EffectRow *e2; } effect_union;
    } data;
} Constraint;

typedef struct ConstraintSet {
    Constraint *constraints;
    size_t count;
    size_t capacity;
} ConstraintSet;
```

### Phase 3.3.2: Inference Engine

```c
typedef struct InferResult {
    Type *type;
    EffectRow *effects;
    ConstraintSet *constraints;
} InferResult;

// Main inference function
InferResult *infer_term(Term *term, Environment *env);

// Helper: infer and add to constraint set
InferResult *infer_with_constraints(Term *term, 
                                   Environment *env,
                                   ConstraintSet *existing);
```

### Phase 3.3.3: Effect Row Solver

```c
typedef struct EffectSubstitution {
    char **effect_vars;      // Effect variable names
    EffectRow **bindings;    // Corresponding effect rows
    size_t count;
} EffectSubstitution;

// Solve effect constraints
EffectSubstitution *solve_effect_constraints(
    ConstraintSet *constraints
);

// Apply substitution to effect row
EffectRow *apply_effect_subst(EffectRow *row, 
                              EffectSubstitution *subst);
```

### Phase 3.3.4: Effect Minimization

```c
// Remove duplicate effects
EffectRow *deduplicate_effects(EffectRow *row);

// Simplify effect row expressions
EffectRow *simplify_effect_row(EffectRow *row);

// Check if effect row is closed (no variables)
bool is_effect_row_closed(EffectRow *row);
```

## Inference Examples

### Example 1: Simple Effect

```scheme
(define get-user [id]
  (perform http-get (format "/users/~a" id)))
```

**Inference trace:**
```
1. infer (perform http-get ...) → (α, {HttpGet}, {})
2. infer (format ...) → (String, {}, {})
3. union: {HttpGet} ∪ {} = {HttpGet}
4. Result: Int -> α / {HttpGet}
```

### Example 2: Effect Composition

```scheme
(define complex-op [x]
  (do
    (perform log "start")
    (let [config (perform read-config)]
      (let [data (perform fetch-data x)]
        (perform log "done")
        data))))
```

**Inference trace:**
```
1. (perform log "start") → (Unit, {Log}, {})
2. (perform read-config) → (Config, {ReadConfig}, {})
3. (perform fetch-data x) → (Data, {FetchData}, {})
4. (perform log "done") → (Unit, {Log}, {})
5. union: {Log} ∪ {ReadConfig} ∪ {FetchData} ∪ {Log}
6. simplify: {Log, ReadConfig, FetchData}
7. Result: a -> Data / {Log, ReadConfig, FetchData}
```

### Example 3: Polymorphic Effects

```scheme
(define twice [f] [x]
  (let [y (f x)]
    (f y)))
```

**Inference trace:**
```
1. f : (a -> b / ε₁)
2. first call (f x) produces effect ε₁
3. second call (f y) produces effect ε₁
4. union: ε₁ ∪ ε₁ = ε₁
5. Result: (a -> b / ε) -> a -> b / ε
```

### Example 4: Handler Effect Removal

```scheme
(handle
  (+ (perform get-x) (perform get-y))
  (get-x [k] (k 10))
  (get-y [k] (k 20)))
```

**Inference trace:**
```
1. body effects: {GetX, GetY}
2. handlers cover: {GetX, GetY}
3. residual: {GetX, GetY} \ {GetX, GetY} = {}
4. Result: Int / {}
```

## Integration with Existing Phases

**Phase 3.1 (Effect Rows):**
- Provides effect row representation
- Defines row operations (union, difference)

**Phase 3.2 (Effect Handlers):**
- Uses inferred effects for handler verification
- Computes residual effects after handling

**Phase 3.3 (This Phase):**
- Automatically generates effect rows
- Enables effect polymorphism
- Minimizes manual annotations

## Test Coverage

Test files validating effect inference:

- `test_effect_inference.omni` - Basic inference patterns
- `test_effect_row.omni` - Row operations during inference
- `test_effects_nested.omni` - Nested effect inference
- `test_effects_compose.omni` - Effect composition
- `test_pure_functions.omni` - Pure function inference

## Success Criteria

- [ ] Constraint generation for all term forms
- [ ] Effect constraint solver
- [ ] Effect row unification
- [ ] Effect polymorphism support
- [ ] Effect row simplification
- [ ] Handler effect removal
- [ ] All inference test files pass
- [ ] No false positives (pure inferred as effectful)
- [ ] No false negatives (effectful inferred as pure)

## Next Phase

**Phase 4.1: Module System** will implement:
- Namespace separation
- Import/export with effect tracking
- Module-level effect signatures

---

**Implementation Approach:** Complete design for HVM type inference engine. C interpreter maintains runtime-only effect dispatch.
