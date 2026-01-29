;; test_define_curried.lisp - Tests for curried function definitions

;; Curried function with two parameters
(define add [x] [y] (+ x y))

;; EXPECT-FINAL: 8
(add 3 5)
