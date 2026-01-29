;; test_foldr.lisp - Tests for foldr (right fold)

;; Fold right: x1 op (x2 op (x3 op (... op init)))
(define foldr [f] [init] [lst]
  (match lst
    ()       init
    (h .. t) (f h (foldr f init t))))

;; TEST: foldr sum (same as foldl for +)
;; EXPECT: 15
(foldr (lambda [x] [acc] (+ x acc)) 0 '(1 2 3 4 5))

;; TEST: foldr subtraction (shows right associativity)
;; 1 - (2 - (3 - (4 - (5 - 0)))) = 1 - (2 - (3 - (4 - 5))) = 1 - (2 - (3 - (-1))) = 1 - (2 - 4) = 1 - (-2) = 3
;; EXPECT: 3
(foldr (lambda [x] [acc] (- x acc)) 0 '(1 2 3 4 5))

;; TEST: foldr on empty list returns init
;; EXPECT: 42
(foldr (lambda [x] [acc] (+ x acc)) 42 '())

;; Build list copy with foldr
;; TEST: list copy via foldr
;; EXPECT: (1 2 3 4 5)
(foldr (lambda [x] [acc] (cons x acc)) '() '(1 2 3 4 5))

;; Append via foldr
(define append-via-foldr [xs] [ys]
  (foldr (lambda [x] [acc] (cons x acc)) ys xs))

;; TEST: append via foldr
;; EXPECT: (1 2 3 4 5 6)
(append-via-foldr '(1 2 3) '(4 5 6))

;; Map via foldr
(define map-via-foldr [f] [lst]
  (foldr (lambda [x] [acc] (cons (f x) acc)) '() lst))

;; TEST: map via foldr
;; EXPECT-FINAL: (2 4 6 8 10)
(map-via-foldr (lambda [x] (* x 2)) '(1 2 3 4 5))
