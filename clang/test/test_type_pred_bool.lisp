;; test_type_pred_bool.lisp - Tests for boolean predicates

;; TEST: bool? with true
;; EXPECT: true
(bool? true)

;; TEST: bool? with false
;; EXPECT: true
(bool? false)

;; TEST: bool? with int
;; EXPECT: false
(bool? 1)

;; TEST: bool? with nil
;; EXPECT: false
(bool? nil)

;; TEST: bool? with string
;; EXPECT: false
(bool? "true")

;; True? checks for exact true value
;; TEST: true? with true
;; EXPECT: true
(true? true)

;; TEST: true? with false
;; EXPECT: false
(true? false)

;; TEST: true? with truthy value
;; EXPECT: false
(true? 1)

;; False? checks for exact false value
;; TEST: false? with false
;; EXPECT: true
(false? false)

;; TEST: false? with true
;; EXPECT: false
(false? true)

;; TEST: false? with nil
;; EXPECT: false
(false? nil)

;; Falsy includes false and nil
(define falsy? [x]
  (or (false? x) (nil? x)))

;; TEST: falsy? false
;; EXPECT: true
(falsy? false)

;; TEST: falsy? nil
;; EXPECT: true
(falsy? nil)

;; TEST: falsy? zero
;; EXPECT: false
(falsy? 0)

;; Truthy is not falsy
(define truthy? [x] (not (falsy? x)))

;; TEST: truthy? true
;; EXPECT: true
(truthy? true)

;; TEST: truthy? string
;; EXPECT: true
(truthy? "")

;; TEST: truthy? nil
;; EXPECT-FINAL: false
(truthy? nil)
