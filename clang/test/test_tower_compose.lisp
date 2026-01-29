;; test_tower_compose.lisp - Tests for tower composition

;; Tower composition combines staged computations
;; TEST: compose lifted
;; EXPECT: 9
(run
  (let [add2 (lift (lambda [x] (+ x 2)))]
  (let [mul3 (lift (lambda [x] (* x 3)))]
    (lift ((run mul3) ((run add2) 1))))))

;; TEST: lift-compose
;; EXPECT: 14
(run
  (lift-compose
    (lift (lambda [x] (+ x 2)))
    (lift (lambda [x] (* x 3)))
    (lift 4)))

;; TEST: compose preserves staging
;; EXPECT: true
(lift?
  (lift-compose
    (lift (lambda [x] x))
    (lift 1)))

;; TEST: sequence of operations
;; EXPECT: (1 4 9)
(run
  (lift-map (lift (lambda [x] (* x x)))
            (lift '(1 2 3))))

;; TEST: tower-bind (monadic)
;; EXPECT: 6
(run
  (tower-bind (lift 3)
    (lambda [x] (lift (* x 2)))))

;; TEST: tower-sequence
;; EXPECT: (1 2 3)
(run
  (tower-sequence
    (list (lift 1) (lift 2) (lift 3))))

;; TEST: tower-ap (applicative)
;; EXPECT: 7
(run
  (tower-ap
    (lift (lambda [a b] (+ a b)))
    (lift 3)
    (lift 4)))

;; TEST: compose at different levels
;; EXPECT: 10
(run (run
  (lift (lift
    (+ 5 5)))))

;; TEST: tower-join (flatten)
;; EXPECT-FINAL: 42
(run
  (tower-join
    (lift (lift 42))))
