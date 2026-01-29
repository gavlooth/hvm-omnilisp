;; test_theorem_induction.lisp - Tests for inductive proofs

;; Induction proves properties over recursive structures
;; TEST: induction on naturals
;; EXPECT: :proved
(theorem sum-formula
  :forall [n {Nat}]
  :prove (= (sum-to n) (/ (* n (+ n 1)) 2))
  :by (induction n
        :base (= (sum-to 0) 0)
        :step (lambda [k ih]
                (= (sum-to (+ k 1))
                   (+ (sum-to k) (+ k 1))))))

;; TEST: induction on lists
;; EXPECT: :proved
(theorem append-length
  :forall [xs {(List a)}] [ys {(List a)}]
  :prove (= (length (append xs ys))
            (+ (length xs) (length ys)))
  :by (induction xs
        :base (= (length (append '() ys)) (length ys))
        :step (lambda [h t ih]
                (= (length (append (cons h t) ys))
                   (+ 1 (length (append t ys)))))))

;; TEST: strong induction
;; EXPECT: :proved
(theorem fib-positive
  :forall [n {Nat}]
  :prove (> (fib n) 0)
  :by (strong-induction n
        :cases
        [(= n 0) (= (fib 0) 1)]
        [(= n 1) (= (fib 1) 1)]
        [:else (> (+ (fib (- n 1)) (fib (- n 2))) 0)]))

;; TEST: structural induction on tree
;; EXPECT: :proved
(theorem tree-size
  :forall [t {(Tree a)}]
  :prove (>= (tree-size t) 1)
  :by (induction t
        :base (= (tree-size (Leaf _)) 1)
        :step (lambda [l v r ih-l ih-r]
                (= (tree-size (Node l v r))
                   (+ 1 (tree-size l) (tree-size r))))))

;; TEST: mutual induction
;; EXPECT: :proved
(theorem even-odd-complement
  :forall [n {Nat}]
  :prove (xor (is-even n) (is-odd n))
  :by (mutual-induction
        [(is-even 0) true]
        [(is-odd 0) false]
        [(is-even (+ k 1)) (is-odd k)]
        [(is-odd (+ k 1)) (is-even k)]))

;; TEST: induction hypothesis use
;; EXPECT-FINAL: :proved
(theorem reverse-reverse
  :forall [xs {(List a)}]
  :prove (= (reverse (reverse xs)) xs)
  :by (induction xs
        :base (= (reverse (reverse '())) '())
        :step (lambda [h t ih]
                (do
                  (assert (= (reverse (reverse t)) t))
                  (= (reverse (reverse (cons h t)))
                     (cons h t))))))
