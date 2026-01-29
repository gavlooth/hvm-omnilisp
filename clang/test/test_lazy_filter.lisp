;; test_lazy_filter.lisp - Tests for lazy filter

;; Lazy-filter returns lazy sequence
;; TEST: lazy-filter creates lazy seq
;; EXPECT: true
(lazy-seq? (lazy-filter (lambda [x] (> x 2)) '(1 2 3 4 5)))

;; TEST: realize lazy-filter
;; EXPECT: (3 4 5)
(realize (lazy-filter (lambda [x] (> x 2)) '(1 2 3 4 5)))

;; TEST: lazy-filter on infinite
;; EXPECT: (0 2 4 6 8)
(take 5 (lazy-filter (lambda [x] (= (% x 2) 0)) (iterate inc 0)))

;; TEST: lazy-filter doesn't evaluate ahead
;; EXPECT: true
(let [evaluated '()]
  (let [lazy (lazy-filter
               (lambda [x]
                 (set! evaluated (cons x evaluated))
                 (> x 2))
               '(1 2 3 4 5))]
    (first lazy)
    (<= (length evaluated) 3)))

;; TEST: lazy-filter all pass
;; EXPECT: (1 2 3)
(realize (lazy-filter (lambda [x] (> x 0)) '(1 2 3)))

;; TEST: lazy-filter none pass
;; EXPECT: ()
(realize (lazy-filter (lambda [x] (< x 0)) '(1 2 3)))

;; TEST: lazy-filter empty
;; EXPECT: ()
(realize (lazy-filter (lambda [x] true) '()))

;; Chain filter and map
;; TEST: lazy filter then map
;; EXPECT: (6 8 10)
(realize
  (lazy-map (lambda [x] (* x 2))
    (lazy-filter (lambda [x] (> x 2)) '(1 2 3 4 5))))

;; TEST: lazy map then filter
;; EXPECT: (4 6 8 10)
(realize
  (lazy-filter (lambda [x] (> x 2))
    (lazy-map (lambda [x] (* x 2)) '(1 2 3 4 5))))

;; TEST: lazy filter on primes
;; EXPECT-FINAL: (2 3 5 7 11)
(define prime? [n]
  (if (<= n 1) false
    (not (some? (lambda [i] (= (% n i) 0))
                (range 2 (+ 1 (floor (sqrt n))))))))
(take 5 (lazy-filter prime? (iterate inc 2)))
