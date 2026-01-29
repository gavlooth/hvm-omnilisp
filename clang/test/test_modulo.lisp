;; test_modulo.lisp - Tests for modulo operator

;; TEST: modulo basic
;; EXPECT: 1
(% 10 3)

;; TEST: modulo even
;; EXPECT: 0
(% 10 5)

;; TEST: modulo smaller dividend
;; EXPECT: 2
(% 2 5)

;; TEST: modulo same
;; EXPECT: 0
(% 5 5)
