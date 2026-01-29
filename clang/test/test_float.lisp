;; test_float.lisp - Tests for floating point numbers

;; Basic float literals
;; TEST: simple float
;; EXPECT: 3.14
3.14

;; TEST: negative float
;; EXPECT: -2.5
-2.5

;; TEST: float with leading dot
;; EXPECT: 0.5
.5

;; Float arithmetic
;; TEST: float addition
;; EXPECT: 5.5
(+ 2.5 3.0)

;; TEST: float subtraction
;; EXPECT: 1.5
(- 4.0 2.5)

;; TEST: float multiplication
;; EXPECT: 6.25
(* 2.5 2.5)

;; TEST: float division
;; EXPECT: 2.5
(/ 5.0 2.0)

;; Mixed int and float
;; TEST: int + float
;; EXPECT: 5.5
(+ 3 2.5)

;; TEST: float / int
;; EXPECT: 2.5
(/ 5.0 2)

;; Float comparison
;; TEST: float less than
;; EXPECT: true
(< 2.5 3.0)

;; TEST: float greater than
;; EXPECT-FINAL: true
(> 3.14 3.0)
