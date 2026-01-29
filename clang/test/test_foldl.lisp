;; test_foldl.lisp - Tests for foldl (left fold / reduce)

;; Fold left: (((init op x1) op x2) op x3) ...
(define foldl [f] [init] [lst]
  (match lst
    ()       init
    (h .. t) (foldl f (f init h) t)))

;; TEST: foldl sum
;; EXPECT: 15
(foldl (lambda [acc] [x] (+ acc x)) 0 '(1 2 3 4 5))

;; TEST: foldl product
;; EXPECT: 120
(foldl (lambda [acc] [x] (* acc x)) 1 '(1 2 3 4 5))

;; TEST: foldl with initial value
;; EXPECT: 100
(foldl (lambda [acc] [x] (+ acc x)) 85 '(1 2 3 4 5))

;; TEST: foldl subtraction (shows left associativity)
;; ((((0 - 1) - 2) - 3) - 4) - 5 = -15
;; EXPECT: -15
(foldl (lambda [acc] [x] (- acc x)) 0 '(1 2 3 4 5))

;; TEST: foldl on empty list returns init
;; EXPECT: 42
(foldl (lambda [acc] [x] (+ acc x)) 42 '())

;; Reduce is foldl with first element as init
(define reduce [f] [lst]
  (match lst
    ()       nothing
    (h .. t) (foldl f h t)))

;; TEST: reduce sum
;; EXPECT: 15
(reduce (lambda [a] [b] (+ a b)) '(1 2 3 4 5))

;; Build reverse with foldl
;; TEST: reverse via foldl
;; EXPECT-FINAL: (5 4 3 2 1)
(foldl (lambda [acc] [x] (cons x acc)) '() '(1 2 3 4 5))
