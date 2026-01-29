;; test_type_pred_nil.lisp - Tests for nil predicates

;; TEST: nil? with nil
;; EXPECT: true
(nil? nil)

;; TEST: nil? with false
;; EXPECT: false
(nil? false)

;; TEST: nil? with zero
;; EXPECT: false
(nil? 0)

;; TEST: nil? with empty string
;; EXPECT: false
(nil? "")

;; TEST: nil? with empty list
;; EXPECT: false
(nil? '())

;; Nothing? is alias for nil?
(define nothing? [x] (nil? x))

;; TEST: nothing? nil
;; EXPECT: true
(nothing? nil)

;; TEST: nothing? value
;; EXPECT: false
(nothing? 42)

;; Some? is opposite of nil?
(define some? [x] (not (nil? x)))

;; TEST: some? with value
;; EXPECT: true
(some? 42)

;; TEST: some? with nil
;; EXPECT: false
(some? nil)

;; TEST: some? with false
;; EXPECT: true
(some? false)

;; TEST: some? with empty list
;; EXPECT: true
(some? '())

;; Safe access with nil check
(define when-some [x] [f]
  (if (some? x) (f x) nil))

;; TEST: when-some with value
;; EXPECT: 10
(when-some 5 (lambda [x] (* x 2)))

;; TEST: when-some with nil
;; EXPECT-FINAL: nil
(when-some nil (lambda [x] (* x 2)))
