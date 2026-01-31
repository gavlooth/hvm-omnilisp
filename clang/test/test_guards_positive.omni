;; test_guards_positive.lisp - Guard with positive number

(define myabs [n]
  (match n
    x & (< x 0)  (- 0 x)
    x            x))

;; EXPECT-FINAL: 42
(myabs 42)
