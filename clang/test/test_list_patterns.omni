;; test_list_patterns.lisp - Tests for list pattern matching

;; Sum using list patterns
(define sum [lst]
  (match lst
    ()       0
    (h .. t) (+ h (sum t))))

;; EXPECT-FINAL: 10
(sum (cons 1 (cons 2 (cons 3 (cons 4 nil)))))
