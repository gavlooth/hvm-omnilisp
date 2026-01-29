;; test_quoted_list.lisp - Tests for quoted list syntax

;; Sum function for testing
(define sum [lst]
  (match lst
    ()       0
    (h .. t) (+ h (sum t))))

;; EXPECT-FINAL: 6
(sum '(1 2 3))
