;; test_guards_classify.lisp - Multiple guards for classification

(define classify [n]
  (match n
    x & (< x 0)   -1
    x & (= x 0)   0
    x & (> x 0)   1))

;; EXPECT-FINAL: 1
(classify 42)
