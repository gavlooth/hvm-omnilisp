;; test_define.lisp - Tests for function definitions

;; Basic function definition and call
(define double [x] (* x 2))

;; EXPECT-FINAL: 10
(double 5)
