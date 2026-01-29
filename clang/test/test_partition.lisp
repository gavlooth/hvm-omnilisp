;; test_partition.lisp - Tests for partition

;; Partition splits list into (matching, non-matching)
(define partition-helper [pred] [lst] [yes] [no]
  (match lst
    ()       (cons (reverse yes) (cons (reverse no) nil))
    (h .. t) (if (pred h)
               (partition-helper pred t (cons h yes) no)
               (partition-helper pred t yes (cons h no)))))

(define partition [pred] [lst]
  (partition-helper pred lst '() '()))

;; TEST: partition evens and odds
;; EXPECT: ((2 4 6) (1 3 5))
(partition (lambda [x] (= (% x 2) 0)) '(1 2 3 4 5 6))

;; TEST: partition positive and non-positive
;; EXPECT: ((1 2 3) (-2 -1 0))
(partition (lambda [x] (> x 0)) '(-2 -1 0 1 2 3))

;; TEST: partition all match
;; EXPECT: ((1 2 3) ())
(partition (lambda [x] (> x 0)) '(1 2 3))

;; TEST: partition none match
;; EXPECT: (() (1 2 3))
(partition (lambda [x] (< x 0)) '(1 2 3))

;; TEST: partition empty
;; EXPECT: (() ())
(partition (lambda [x] true) '())

;; Separate is alias
(define separate [pred] [lst] (partition pred lst))

;; TEST: separate
;; EXPECT-FINAL: ((2 4) (1 3 5))
(separate (lambda [x] (= (% x 2) 0)) '(1 2 3 4 5))
