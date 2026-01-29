;; test_clambda.lisp - Tests for code-generating lambda

;; Clambda creates staged functions
;; TEST: basic clambda
;; EXPECT: 6
(let [f (clambda [x] (+ x 1))]
  (run (f (lift 5))))

;; TEST: clambda with multiple args
;; EXPECT: 7
(let [f (clambda [a b] (+ a b))]
  (run (f (lift 3) (lift 4))))

;; TEST: clambda composition
;; EXPECT: 12
(let [double (clambda [x] (* x 2))]
(let [add3 (clambda [x] (+ x 3))]
  (run (double (add3 (lift 3))))))

;; TEST: clambda generates code
;; EXPECT: true
(let [f (clambda [x] (* x 2))]
  (lift? (f (lift 5))))

;; TEST: clambda with staged body
;; EXPECT: 25
(let [square (clambda [x] (lift (* (splice x) (splice x))))]
  (run (square (lift 5))))

;; TEST: clambda partial application
;; EXPECT: 10
(let [add (clambda [a] [b] (+ a b))]
  (let [add5 (add (lift 5))]
    (run (add5 (lift 5)))))

;; TEST: clambda with free vars
;; EXPECT: 15
(let [n 10]
  (let [addn (clambda [x] (+ x n))]
    (run (addn (lift 5)))))

;; TEST: nested clambda
;; EXPECT: 8
(let [f (clambda [x]
          (clambda [y] (+ x y)))]
  (run ((f (lift 3)) (lift 5))))

;; TEST: clambda in tower-map
;; EXPECT-FINAL: (2 4 6)
(let [double (clambda [x] (* x 2))]
  (run (tower-map double (lift '(1 2 3)))))
