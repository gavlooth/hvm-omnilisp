;; test_length.lisp - Tests for list length with patterns

(define len [lst]
  (match lst
    ()       0
    (h .. t) (+ 1 (len t))))

;; EXPECT-FINAL: 5
(len (cons 1 (cons 2 (cons 3 (cons 4 (cons 5 nil))))))
