# Phase 3.1: Effect Row Types

**Status:** Design Phase  
**Date:** 2026-02-05

## Overview

Effect row types extend the type system to statically track which effects a computation may perform. This enables compile-time verification that all effects are properly handled and allows for effect polymorphism.

## Motivation

Current state (Phase 2.4):
- Type inference with higher-kinded types ✅
- Runtime effect handling via `handle`/`perform` ✅
- No static tracking of effects ❌

With effect row types, we can:
- Statically verify all performed effects have handlers
- Enable effect polymorphism (functions work with any effect set)
- Provide clear effect signatures in function types
- Catch missing handlers at compile time

## Type System Extension

### Effect Row Syntax

```scheme
;; Pure function (no effects)
add : Int -> Int -> Int / {}

;; Function with State effect
get-counter : Unit -> Int / {State}

;; Function with multiple effects
complex : Int -> String / {State, IO, Log}

;; Effect-polymorphic function
map : (a -> b / ε) -> List a -> List b / ε
```

### Row Types

Effect rows are extensible records of effect labels:

```
ε ::= {}                    -- empty row (pure)
    | {E}                   -- single effect
    | {E₁, E₂, ..., Eₙ}     -- effect union
    | ε₁ ∪ ε₂               -- row union
    | ε₁ \ E                -- row difference (remove effect)
    | α                     -- effect variable
```

### Type Inference Rules

**Pure Expression:**
```
Γ ⊢ e : τ / {}
```

**Effect Perform:**
```
perform E p : τ / {E}
```

**Effect Handle:**
```
Γ ⊢ e : τ / {E} ∪ ε
Γ ⊢ handler for E
─────────────────────
Γ ⊢ (handle e handler) : τ / ε
```

**Function Application:**
```
Γ ⊢ f : (τ₁ -> τ₂ / ε₁)
Γ ⊢ x : τ₁ / ε₂
─────────────────────
Γ ⊢ (f x) : τ₂ / ε₁ ∪ ε₂
```

**Let Binding:**
```
Γ ⊢ e₁ : τ₁ / ε₁
Γ, x:τ₁ ⊢ e₂ : τ₂ / ε₂
─────────────────────────
Γ ⊢ (let [x e₁] e₂) : τ₂ / ε₁ ∪ ε₂
```

## Implementation Strategy

### Phase 3.1.1: Effect Row Type Representation

Add effect row types to the type system:

```c
typedef enum {
    TYPE_INT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_FUNCTION,
    TYPE_VAR,
    TYPE_CON,
    TYPE_APP,
    TYPE_EFFECT_ROW  // NEW
} TypeKind;

typedef struct EffectRow {
    char **effects;      // Array of effect names
    size_t count;        // Number of effects
    bool is_open;        // true if row has effect variable
    char *row_var;       // Effect variable name (if is_open)
} EffectRow;

typedef struct FunctionType {
    Type *arg_type;
    Type *ret_type;
    EffectRow *effects;  // NEW: effect row
} FunctionType;
```

### Phase 3.1.2: Effect Row Operations

Implement core row operations:

```c
// Create empty effect row (pure)
EffectRow *effect_row_empty();

// Create row with single effect
EffectRow *effect_row_singleton(const char *effect);

// Union of two effect rows
EffectRow *effect_row_union(EffectRow *r1, EffectRow *r2);

// Remove effect from row
EffectRow *effect_row_remove(EffectRow *row, const char *effect);

// Check if row contains effect
bool effect_row_contains(EffectRow *row, const char *effect);

// Check if one row is a subset of another
bool effect_row_subset(EffectRow *r1, EffectRow *r2);
```

### Phase 3.1.3: Type Inference Integration

Extend unification to handle effect rows:

```c
// Unify two effect rows
bool unify_effect_rows(EffectRow *r1, EffectRow *r2, 
                       Substitution *subst);

// Infer effects for expression
EffectRow *infer_effects(Term *term, Environment *env);
```

### Phase 3.1.4: Effect Checking

Add compile-time effect verification:

```c
// Check that all performed effects are handled
bool check_effects_handled(Term *body, EffectRow *handlers);

// Verify handle expression completeness
bool verify_handle(Term *handle_expr);
```

## Test Coverage

The following test files validate effect row types:

- `test_effect_row.omni` - Basic effect operations
- `test_effect_rows.omni` - Row operations (union, difference)
- `test_effect_row_types.omni` - Type-level effect tracking
- `test_effect_inference.omni` - Automatic effect inference

## Examples

### Example 1: Pure Function

```scheme
(define add [x] [y]
  (+ x y))

;; Inferred type: Int -> Int -> Int / {}
```

### Example 2: Single Effect

```scheme
(define get-config []
  (perform read-config))

;; Inferred type: Unit -> Config / {ReadConfig}
```

### Example 3: Effect Polymorphism

```scheme
(define map [f] [lst]
  (match lst
    () ()
    (h .. t) (cons (f h) (map f t))))

;; Inferred type: (a -> b / ε) -> List a -> List b / ε
;; The function preserves whatever effects f has
```

### Example 4: Effect Handling

```scheme
(handle
  (+ (perform get-x) (perform get-y))
  (get-x [resume] (resume 10))
  (get-y [resume] (resume 20)))

;; Inside handle body: Int / {GetX, GetY}
;; After handling: Int / {}
```

### Example 5: Effect Composition

```scheme
(define complex-operation [x]
  (let [config (perform read-config)]
    (let [data (perform fetch-data x)]
      (perform log (format "Processing ~a" data))
      (+ (length data) (parse-int config)))))

;; Inferred: Int -> Int / {ReadConfig, FetchData, Log}
```

## Implementation Notes

### HVM vs C Interpreter

**HVM Implementation:**
- Full effect row type checking
- Effect polymorphism
- Compile-time verification

**C Interpreter (Optional):**
- Could add basic effect tracking for warnings
- Runtime effect verification only
- No static polymorphism

Given HEARTBEAT.md marks this as "HVM only", the C interpreter will continue with runtime-only effect handling. The HVM backend will get full static effect tracking.

## Success Criteria

- [ ] Effect row type representation in type system
- [ ] Row operations (union, difference, subset)
- [ ] Type inference generates effect rows
- [ ] Unification handles effect rows
- [ ] Test suite validates effect tracking
- [ ] Effect polymorphism works correctly

## Next Phase

**Phase 3.2: Effect Handlers** will implement:
- Handler synthesis from type information
- Multi-shot continuations
- Effect composition patterns

---

**Implementation Approach:** Design-first for HVM backend. C interpreter maintains current runtime-only effect handling.
