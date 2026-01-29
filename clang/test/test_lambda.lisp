;; test_lambda.lisp - Tests for lambda expressions

;; Basic lambda with single parameter
(define apply-fn [f] [x] (f x))

;; TEST: lambda keyword
;; EXPECT: 25
(apply-fn (lambda [x] (* x x)) 5)

;; TEST: fn keyword (alias for lambda)
;; EXPECT: 16
(apply-fn (fn [x] (* x x)) 4)

;; TEST: lambda with multiple operations
;; EXPECT: 7
(apply-fn (lambda [x] (+ x 2)) 5)

;; TEST: nested lambda application
(define make-adder [n] (lambda [x] (+ x n)))
(define add5 (make-adder 5))

;; EXPECT-FINAL: 15
(add5 10)
