;; test_theorem_prove.lisp - Tests for theorem proving

;; Define propositions
;; TEST: define proposition
;; EXPECT: true
(defprop identity-prop
  (forall [x] (= x x)))
(prop? identity-prop)

;; TEST: prove trivial
;; EXPECT: :proven
(prove identity-prop)

;; TEST: prove by computation
;; EXPECT: :proven
(defprop add-comm
  (forall [a b] (= (+ a b) (+ b a))))
(prove add-comm :strategy :compute)

;; TEST: prove with witness
;; EXPECT: :proven
(defprop exists-positive
  (exists [n] (> n 0)))
(prove exists-positive :witness 1)

;; TEST: prove implication
;; EXPECT: :proven
(defprop modus-ponens
  (forall [P Q]
    (-> (and P (-> P Q)) Q)))
(prove modus-ponens)

;; TEST: unproven returns unproven
;; EXPECT: :unproven
(defprop false-prop
  (forall [x] (= x (+ x 1))))
(prove false-prop :timeout 100)

;; Inductive proof
;; TEST: prove by induction
;; EXPECT: :proven
(defprop nat-induction
  (forall [P]
    (-> (and (P 0)
             (forall [n] (-> (P n) (P (+ n 1)))))
        (forall [n] (P n)))))
(prove nat-induction :strategy :induction)

;; TEST: proof term
;; EXPECT: true
(let [pf (prove identity-prop)]
  (proof? pf))

;; TEST: extract witness
;; EXPECT-FINAL: 1
(let [pf (prove exists-positive :witness 1)]
  (proof-witness pf))
