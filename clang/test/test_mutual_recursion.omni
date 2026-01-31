;; test_mutual_recursion.lisp - Tests for mutually recursive functions

;; Even/odd mutual recursion
(define iseven [n]
  (if (= n 0)
    1
    (isodd (- n 1))))

(define isodd [n]
  (if (= n 0)
    0
    (iseven (- n 1))))

;; EXPECT-FINAL: 1
(iseven 10)
