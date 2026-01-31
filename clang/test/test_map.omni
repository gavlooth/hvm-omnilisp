;; test_map.lisp - Tests for map with list patterns

(define map [f] [lst]
  (match lst
    ()       nil
    (h .. t) (cons (f h) (map f t))))

(define double [x] (* x 2))

(define sum [lst]
  (match lst
    ()       0
    (h .. t) (+ h (sum t))))

;; Map doubles the list, sum adds them up: (2+4+6) = 12
;; EXPECT-FINAL: 12
(sum (map double (cons 1 (cons 2 (cons 3 nil)))))
