;; test_assert_eq.lisp - Tests for assert-eq equality assertions

;; Assert-eq checks structural equality
;; TEST: equal integers
;; EXPECT: true
(assert-eq 5 5)

;; TEST: equal strings
;; EXPECT: true
(assert-eq "hello" "hello")

;; TEST: equal lists
;; EXPECT: true
(assert-eq '(1 2 3) '(1 2 3))

;; TEST: equal nested
;; EXPECT: true
(assert-eq '(1 (2 3) 4) '(1 (2 3) 4))

;; TEST: equal dicts
;; EXPECT: true
(assert-eq #{"a" 1 "b" 2} #{"b" 2 "a" 1})

;; TEST: not equal throws
;; EXPECT: :assertion-failed
(handle
  (assert-eq 5 6)
  (AssertionError [e k] :assertion-failed))

;; TEST: assert-eq with message
;; EXPECT: :caught
(handle
  (assert-eq 1 2 "values should be equal")
  (AssertionError [e k] :caught))

;; TEST: assert-not-eq
;; EXPECT: true
(assert-not-eq 5 6)

;; TEST: assert-not-eq fails on equal
;; EXPECT: :failed
(handle
  (assert-not-eq 5 5)
  (AssertionError [e k] :failed))

;; TEST: assert-eq nil
;; EXPECT: true
(assert-eq nil nil)

;; TEST: assert-eq types matter
;; EXPECT: :failed
(handle
  (assert-eq 5 "5")
  (AssertionError [e k] :failed))

;; TEST: assert-eq custom types
;; EXPECT-FINAL: true
(do
  (defrecord Point [x] [y])
  (assert-eq (Point 1 2) (Point 1 2)))
