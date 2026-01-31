;; test_pattern_literals.lisp - Tests for literal pattern matching

;; TEST: match literal 0
;; EXPECT: 100
(match 0 0 100 _ 0)

;; TEST: match literal 5
;; EXPECT: 500
(match 5 5 500 _ 0)

;; TEST: wildcard fallback
;; EXPECT: 999
(match 42 0 100 1 200 _ 999)
