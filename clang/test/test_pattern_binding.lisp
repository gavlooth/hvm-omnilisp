;; test_pattern_binding.lisp - Tests for variable binding in patterns

;; TEST: bind single variable
;; EXPECT: 42
(match 42 x x)

;; TEST: bind and compute
;; EXPECT: 84
(match 42 x (* x 2))
