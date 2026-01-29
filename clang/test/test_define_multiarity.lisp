;; test_define_multiarity.lisp - Tests for multi-arity function definitions

;; Multi-arity via pattern matching on argument structure
(define greet
  [()]           "Hello, World!"
  [(name)]       (cons "Hello, " (cons name nil))
  [(first last)] (cons "Hello, " (cons first (cons " " (cons last nil)))))

;; TEST: zero-arity
;; EXPECT: "Hello, World!"
(greet)

;; Factorial with base case pattern
(define fact
  [(0)]  1
  [(n)]  (* n (fact (- n 1))))

;; TEST: factorial of 5
;; EXPECT: 120
(fact 5)

;; Fibonacci with multiple base cases
(define fib
  [(0)]  0
  [(1)]  1
  [(n)]  (+ (fib (- n 1)) (fib (- n 2))))

;; TEST: fibonacci of 10
;; EXPECT-FINAL: 55
(fib 10)
