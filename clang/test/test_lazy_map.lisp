;; test_lazy_map.lisp - Tests for lazy map

;; Lazy-map returns lazy sequence
;; TEST: lazy-map creates lazy seq
;; EXPECT: true
(lazy-seq? (lazy-map (lambda [x] (* x 2)) '(1 2 3)))

;; TEST: realize lazy-map
;; EXPECT: (2 4 6)
(realize (lazy-map (lambda [x] (* x 2)) '(1 2 3)))

;; TEST: lazy-map on infinite
;; EXPECT: (0 2 4 6 8)
(take 5 (lazy-map (lambda [x] (* x 2)) (iterate inc 0)))

;; TEST: lazy-map doesn't compute ahead
;; EXPECT: true
(let [computed 0]
  (let [lazy (lazy-map (lambda [x]
                         (set! computed (+ computed 1))
                         (* x 2))
                       '(1 2 3 4 5))]
    (first lazy)  ;; compute only first
    (= computed 1)))

;; TEST: lazy-map empty
;; EXPECT: ()
(realize (lazy-map (lambda [x] (* x 2)) '()))

;; Chained lazy operations
;; TEST: lazy-map chain
;; EXPECT: (4 8 12)
(realize
  (lazy-map (lambda [x] (* x 2))
    (lazy-map (lambda [x] (* x 2)) '(1 2 3))))

;; TEST: lazy-map preserves laziness through chain
;; EXPECT: (2 4 6 8 10)
(take 5
  (lazy-map (lambda [x] (* x 2))
    (lazy-map (lambda [x] (+ x 1))
      (iterate (lambda [x] (+ x 1)) 0))))

;; Lazy-map with index
;; TEST: lazy-map-indexed
;; EXPECT: ((0 "a") (1 "b") (2 "c"))
(realize (lazy-map-indexed (lambda [i x] (list i x)) '("a" "b" "c")))

;; TEST: lazy-map-indexed infinite
;; EXPECT-FINAL: ((0 0) (1 1) (2 2) (3 3) (4 4))
(take 5 (lazy-map-indexed (lambda [i x] (list i x)) (iterate inc 0)))
