# OmniLisp Theorem Proving Design

## Overview

This document explores how to add theorem proving capabilities to OmniLisp by capitalizing on HVM4's unique computational model.

## HVM4 Properties for Theorem Proving

### 1. Superposition (SUP) for Proof Search
HVM4's SUP construct creates superpositions that evaluate in parallel:
```hvm4
&{branch1, branch2}  // Both branches explored simultaneously
```

This maps directly to **non-deterministic proof search**:
- Each SUP branch represents a proof attempt
- HVM4 explores all branches in parallel
- First successful proof "wins"
- Failed branches are pruned automatically

### 2. Interaction Nets ≈ Proof Nets
Girard's Linear Logic proof nets and Lafont's Interaction Nets share deep structure:
- Both are graph-based
- Both have local rewrite rules
- Cut elimination ≈ Interaction reduction

This means **HVM4 reductions ARE proof normalization**.

### 3. Optimal Sharing for Proof Terms
When proofs share subterms (common in dependent types), HVM4's optimal sharing ensures:
- No redundant proof computation
- Proof terms automatically memoized
- Exponentially smaller proof representations possible

---

## Approach 1: Refinement Types (Recommended First Step)

Refinement types add logical predicates to base types without full dependent types.

### Syntax
```lisp
;; Type with refinement predicate
(define divide [x {Int}] [y {Int | (not (= y 0))}] {Int}
  (/ x y))

;; Refinement on result
(define abs [x {Int}] {Int | (>= result 0)}
  (if (< x 0) (- 0 x) x))

;; Compound refinements
(define sqrt [x {Float | (>= x 0.0)}] {Float | (>= result 0.0)}
  ...)
```

### Implementation
1. **Parse refinement predicates** into AST
2. **Generate verification conditions (VCs)** at compile time
3. **Use SUP to search for counterexamples** (property-based testing on steroids)
4. **Optional: SMT solver integration** for decidable fragments

### HVM4 Advantage
```hvm4
// Check refinement via SUP-powered search
@check_refinement = λpredicate. λvalue.
  // Generate test cases using superposition
  !!&tests = @generate_test_cases(value);
  @all_satisfy(predicate)(tests)

// SUP explores all test cases in parallel
@generate_test_cases = λv.
  &{v, &{(v + 1), &{(v - 1), &{0, &{-1, ...}}}}}
```

---

## Approach 2: Dependent Types (Full Power)

Full dependent types where types can depend on values.

### Core Calculus
```
Types:
  τ ::= Int | Bool | (Π (x : τ₁) τ₂) | (Σ (x : τ₁) τ₂) | (= e₁ e₂) | Type_n

Terms:
  e ::= x | λx:τ.e | e₁ e₂ | (e₁, e₂) | fst e | snd e | refl | J | ...
```

### OmniLisp Syntax
```lisp
;; Pi type (dependent function)
(define-type (Vec A n)
  [nil  (Vec A 0)]
  [cons [x A] [xs (Vec A n)] (Vec A (+ n 1))])

;; Dependent function
(define head [A Type] [n {Nat | (> n 0)}] [v (Vec A n)] A
  (match v
    [(cons x _) x]))

;; Equality type and proof
(define plus-zero-right [n Nat] (= (+ n 0) n)
  (match n
    [0 refl]
    [(succ m) (cong succ (plus-zero-right m))]))
```

### Type Checking as Evaluation
Key insight: **Type checking in dependent types = evaluation**

HVM4's evaluator can be used for:
1. Normalizing type expressions
2. Checking type equality (definitional equality)
3. Proof term reduction

```hvm4
// Type equality check via normalization
@types_equal = λt1. λt2.
  !!&n1 = @normalize(t1);
  !!&n2 = @normalize(t2);
  @alpha_equiv(n1)(n2)

// HVM4 normalizes efficiently with optimal sharing
@normalize = λt. @omni_eval(@omni_menv_empty)(t)
```

---

## Approach 3: Tactics & Proof Search (Leveraging SUP)

Use HVM4's SUP for automatic proof search with tactics.

### Tactic Language
```lisp
;; Prove a theorem using tactics
(theorem plus-comm [m Nat] [n Nat] (= (+ m n) (+ n m))
  (induction m
    [base (rewrite plus-zero-right) refl]
    [step (rewrite plus-succ-right) (cong succ IH)]))

;; Auto-tactic using SUP
(theorem simple-fact [x Nat] (= (+ x 0) x)
  auto)  ; SUP explores proof space
```

### SUP-Powered Auto Tactic
```hvm4
// Auto-tactic explores multiple strategies in parallel
@auto_prove = λgoal. λctx.
  &Auto{
    @try_refl(goal),
    &{@try_assumption(goal)(ctx),
    &{@try_intro(goal)(ctx),
    &{@try_elim(goal)(ctx),
    &{@try_rewrite(goal)(ctx),
    @try_induction(goal)(ctx)}}}}
  }

// Each branch is a proof attempt
// HVM4 explores all in parallel
// First success returns the proof term
```

### Proof Search as Superposition
```hvm4
// Prolog-style logic programming via SUP
@unify = λt1. λt2.
  λ{
    // Variable unifies with anything
    #Var: λ&v. #Subst{v, t2}

    // Constructor heads must match
    #App: λ&f1. λ&a1.
      λ{
        #App: λ&f2. λ&a2.
          !!&s1 = @unify(f1)(f2);
          !!&s2 = @unify(@apply_subst(s1)(a1))(@apply_subst(s1)(a2));
          @compose_subst(s1)(s2)
        _: #Fail
      }(t2)

    _: λ{1: #Empty; _: #Fail}((t1 == t2))
  }(t1)

// Resolution with SUP for backtracking
@resolve = λgoal. λclauses.
  @choice_map(λclause.
    !!&subst = @unify(goal)(@head(clause));
    λ{
      #Fail: #Fail
      _: @resolve_all(@apply_subst(subst)(@body(clause)))(clauses)
    }(subst)
  )(clauses)

// choice_map uses SUP for non-determinism
@choice_map = λf. λxs.
  λ{
    #NIL: #Fail
    #CON: λ&h. λ&t. &Choice{f(h), @choice_map(f)(t)}
  }(xs)
```

---

## Approach 4: Linear Types (Native to HVM4)

HVM4's DUP/SUP semantics are inherently linear. We can expose this.

### Linear Type Syntax
```lisp
;; Linear function (argument used exactly once)
(define-linear use-once [x {File -o Unit}]
  (close x))

;; Unrestricted (can be used multiple times)
(define-unrestricted shared [x {!Int -> Int}]
  (+ x x))

;; Session types for protocols
(define-session Client
  (send Int)
  (recv Bool)
  end)
```

### HVM4 Linearity Checking
```hvm4
// At compile time, track variable usage
// HVM4's DUP nodes indicate non-linear use
// If no DUP needed, type is linear

@check_linearity = λterm. λlinear_vars.
  λ{
    #Var: λ&i.
      !!&is_linear = @set_member(linear_vars)(i);
      λ{
        1: @mark_used(i)  // Linear var, mark as used
        _: #Ok           // Unrestricted, ok
      }(is_linear)

    #App: λ&f. λ&a.
      // Check both, ensure linear vars used exactly once
      !!&r1 = @check_linearity(f)(linear_vars);
      !!&r2 = @check_linearity(a)(linear_vars);
      @merge_linear_checks(r1)(r2)

    // DUP indicates variable used twice - error if linear
    #DUP: λ&v. λ&body.
      !!&is_linear = @set_member(linear_vars)(v);
      λ{
        1: #Err{#linear_used_twice, v}
        _: @check_linearity(body)(linear_vars)
      }(is_linear)

    ...
  }(term)
```

---

## Approach 5: Proof-Relevant Programming

Proofs are first-class values that can be inspected and manipulated.

### Design
```lisp
;; Proofs are values
(define-theorem plus-assoc
  (forall [a Nat] [b Nat] [c Nat]
    (= (+ (+ a b) c) (+ a (+ b c)))))

;; Use proof at runtime
(define transport [A Type] [B Type] [pf (= A B)] [x A] B
  (J (λ_ (λT T)) x pf))

;; Proof-carrying code
(define safe-head [A Type] [xs (List A)] [pf (not (= xs nil))] A
  ...)
```

### HVM4 Representation
```hvm4
// Proof terms are HVM4 terms
// Normalization = proof simplification

#Refl{a}           // Reflexivity proof: a = a
#Sym{pf}           // Symmetry: if pf : a = b, then b = a
#Trans{pf1, pf2}   // Transitivity: a = b, b = c => a = c
#Cong{f, pf}       // Congruence: a = b => f(a) = f(b)
#J{motive, base, pf}  // Eliminator for equality

// Proof normalization via HVM4 evaluation
@normalize_proof = λpf.
  λ{
    #Trans: λ&p1. λ&p2.
      !!&n1 = @normalize_proof(p1);
      !!&n2 = @normalize_proof(p2);
      λ{
        #Refl: λ&_. n2  // refl ∘ p = p
        _: λ{
          #Refl: λ&_. n1  // p ∘ refl = p
          _: #Trans{n1, n2}
        }(n2)
      }(n1)
    ...
  }(pf)
```

---

## Implementation Roadmap

### Phase 1: Refinement Types (3-4 weeks)
1. Extend parser for refinement predicates `{T | P}`
2. Add VC generation pass
3. Implement SUP-powered counterexample search
4. Optional: Z3/CVC5 integration for SMT solving

### Phase 2: Proof-Relevant Effects (2-3 weeks)
1. Extend effect system with proof obligations
2. `(perform verify predicate)` effect
3. Handlers can provide proofs or search for them

### Phase 3: Linear Types (2-3 weeks)
1. Add linearity annotations `-o` vs `->`
2. Leverage HVM4's DUP tracking for checking
3. Session types for protocols

### Phase 4: Dependent Types (4-6 weeks)
1. Add Pi and Sigma types
2. Universe hierarchy
3. Equality type with J eliminator
4. Proof irrelevance optimization

### Phase 5: Tactics & Automation (3-4 weeks)
1. Tactic DSL
2. SUP-powered auto tactic
3. Proof search strategies
4. Integration with macro system

---

## Example: Complete Verified Program

```lisp
;; Import theorem proving prelude
(import theorem-proving)

;; Define natural numbers with induction principle
(define-inductive Nat
  [zero Nat]
  [succ [n Nat] Nat])

;; Define addition with termination proof (structural recursion)
(define/verified plus [m Nat] [n Nat] Nat
  :termination (< m)
  :spec (and (= (plus zero n) n)
             (= (plus (succ m) n) (succ (plus m n))))
  (match m
    [zero n]
    [(succ m') (succ (plus m' n))]))

;; Prove commutativity
(theorem plus-comm
  :statement (forall [m Nat] [n Nat] (= (plus m n) (plus n m)))
  :proof
  (induction m
    [zero
     ;; Goal: (= (plus zero n) (plus n zero))
     ;; By spec: (plus zero n) = n
     ;; Need: (= n (plus n zero))
     (symmetry (plus-zero-right n))]
    [(succ m')
     ;; IH: (= (plus m' n) (plus n m'))
     ;; Goal: (= (plus (succ m') n) (plus n (succ m')))
     ;; By spec: (plus (succ m') n) = (succ (plus m' n))
     ;; Need: (= (succ (plus m' n)) (plus n (succ m')))
     (chain
       (cong succ IH)                    ; (succ (plus m' n)) = (succ (plus n m'))
       (symmetry (plus-succ-right n m')))]))  ; (succ (plus n m')) = (plus n (succ m'))

;; Use the theorem
(define example [x Nat] [y Nat] (= (plus x y) (plus y x))
  (plus-comm x y))  ; Returns proof term
```

---

## Key Insight: HVM4 as Proof Engine

The deepest insight is that **HVM4's reduction IS proof normalization**:

1. **Terms = Proofs** (Curry-Howard)
2. **Reduction = Cut Elimination** (Proof simplification)
3. **SUP = Proof Search** (Non-deterministic exploration)
4. **Optimal Sharing = Proof Compression** (No redundant subproofs)

HVM4 isn't just running our programs—it's already a proof engine. We just need to expose this capability through the type system.

---

## References

- Amin & Rompf, "Collapsing Towers of Interpreters" (POPL 2018) - staging
- Girard, "Linear Logic" (1987) - proof nets
- Lafont, "Interaction Combinators" (1997) - interaction nets
- Norell, "Towards a Practical Programming Language Based on Dependent Type Theory" (2007) - Agda
- Vazou et al., "Liquid Haskell" (2014) - refinement types
- Brady, "Idris 2: Quantitative Type Theory in Practice" (2021) - linear dependent types
