;; test_arithmetic.lisp - Tests for arithmetic operations

;; TEST: addition of two numbers
;; EXPECT: 5
(+ 2 3)

;; TEST: subtraction
;; EXPECT: 7
(- 10 3)

;; TEST: multiplication
;; EXPECT: 24
(* 4 6)

;; TEST: division
;; EXPECT: 5
(/ 15 3)

;; TODO: modulo not yet implemented
;; TEST: modulo
;; EXPECT: SKIP
;; (% 10 3)

;; TEST: nested arithmetic
;; EXPECT: 14
(+ (* 2 3) (* 2 4))

;; TEST: deeply nested
;; EXPECT: 100
(* (+ 2 3) (* 4 5))

;; TODO: negative numbers wrap around (unsigned 32-bit)
;; TEST: negative result
;; EXPECT: SKIP
;; (- 5 10)

;; TEST: zero
;; EXPECT: 0
(- 5 5)
