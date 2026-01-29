;; test_type_pred_float.lisp - Tests for float and number predicates

;; TEST: float? with float
;; EXPECT: true
(float? 3.14)

;; TEST: float? with int
;; EXPECT: false
(float? 42)

;; TEST: float? with string
;; EXPECT: false
(float? "3.14")

;; TEST: float? with zero float
;; EXPECT: true
(float? 0.0)

;; TEST: float? with negative
;; EXPECT: true
(float? -2.5)

;; Number? returns true for int or float
;; TEST: number? with int
;; EXPECT: true
(number? 42)

;; TEST: number? with float
;; EXPECT: true
(number? 3.14)

;; TEST: number? with string
;; EXPECT: false
(number? "42")

;; TEST: number? with nil
;; EXPECT: false
(number? nil)

;; TEST: number? with list
;; EXPECT: false
(number? '(1 2 3))

;; Numeric predicates
(define positive? [x] (and (number? x) (> x 0)))
(define negative? [x] (and (number? x) (< x 0)))
(define zero? [x] (and (number? x) (= x 0)))

;; TEST: positive?
;; EXPECT: true
(positive? 5)

;; TEST: negative?
;; EXPECT: true
(negative? -3)

;; TEST: zero?
;; EXPECT-FINAL: true
(zero? 0)
