;; test_higher_order.lisp - Tests for higher-order functions

;; Function as argument
(define apply-twice [f] [x] (f (f x)))
(define inc [x] (+ x 1))

;; TEST: apply function twice
;; EXPECT: 7
(apply-twice inc 5)

;; Function as return value
(define compose [f] [g]
  (lambda [x] (f (g x))))

(define double [x] (* x 2))
(define square [x] (* x x))

;; TEST: compose double and inc: double(inc(3)) = double(4) = 8
(define double-after-inc (compose double inc))

;; EXPECT: 8
(double-after-inc 3)

;; TEST: compose square and double: square(double(3)) = square(6) = 36
(define square-after-double (compose square double))

;; EXPECT: 36
(square-after-double 3)

;; Passing multiple functions
(define apply-both [f] [g] [x] (+ (f x) (g x)))

;; TEST: apply both inc and double to 5: (5+1) + (5*2) = 6 + 10 = 16
;; EXPECT-FINAL: 16
(apply-both inc double 5)
