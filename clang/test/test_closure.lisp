;; test_closure.lisp - Tests for lexical closures

;; Closure captures enclosing variable
(define make-counter [start]
  (lambda [n] (+ start n)))

(define counter-from-10 (make-counter 10))

;; TEST: closure captures start=10
;; EXPECT: 15
(counter-from-10 5)

;; TEST: different closure instance captures start=100
(define counter-from-100 (make-counter 100))

;; EXPECT: 105
(counter-from-100 5)

;; Nested closures
(define make-multiplier [x]
  (lambda [y]
    (lambda [z] (* (* x y) z))))

(define mul-by-2 (make-multiplier 2))
(define mul-by-2-and-3 (mul-by-2 3))

;; TEST: nested closure captures x=2, y=3, applies z=4: 2*3*4 = 24
;; EXPECT-FINAL: 24
(mul-by-2-and-3 4)
