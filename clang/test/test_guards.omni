;; test_guards.lisp - Tests for guard patterns in match

;; Max of two numbers using guards
(define mymax [a] [b]
  (match a
    x & (> x b)  x
    _            b))

;; EXPECT-FINAL: 10
(mymax 10 5)
