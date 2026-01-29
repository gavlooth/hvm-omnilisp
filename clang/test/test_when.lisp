;; test_when.lisp - Tests for when conditional

;; TEST: when true returns body
;; EXPECT: 42
(when true 42)

;; TEST: when false returns nothing
;; EXPECT: #Noth{}
(when false 42)
