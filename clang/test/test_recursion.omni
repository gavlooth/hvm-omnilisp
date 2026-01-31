;; test_recursion.lisp - Tests for recursive functions

;; Factorial
(define fact [n]
  (if (= n 0)
    1
    (* n (fact (- n 1)))))

;; EXPECT-FINAL: 3628800
(fact 10)
