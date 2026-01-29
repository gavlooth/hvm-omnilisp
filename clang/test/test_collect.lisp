;; test_collect.lisp - Tests for collect (realize lazy to list)

;; Collect forces lazy sequence to list
;; TEST: collect lazy range
;; EXPECT: (0 1 2 3 4)
(collect (take 5 (iterate inc 0)))

;; TEST: collect already list
;; EXPECT: (1 2 3)
(collect '(1 2 3))

;; TEST: collect empty
;; EXPECT: ()
(collect '())

;; TEST: collect lazy-map
;; EXPECT: (2 4 6)
(collect (lazy-map (lambda [x] (* x 2)) '(1 2 3)))

;; TEST: collect lazy-filter
;; EXPECT: (2 4)
(collect (lazy-filter (lambda [x] (= (% x 2) 0)) '(1 2 3 4 5)))

;; TEST: collect chained lazy ops
;; EXPECT: (4 8)
(collect
  (lazy-map (lambda [x] (* x 2))
    (lazy-filter (lambda [x] (= (% x 2) 0))
      (take 5 (iterate inc 1)))))

;; Realize is alias for collect
;; TEST: realize
;; EXPECT: (1 2 3 4 5)
(realize (take 5 (iterate inc 1)))

;; Into collects into specific container
;; TEST: into vector
;; EXPECT: [1 2 3]
(into [] '(1 2 3))

;; TEST: into set
;; EXPECT: #{1 2 3}
(into #{} '(1 2 3 2 1))

;; TEST: into existing
;; EXPECT: [0 1 2 3]
(into [0] '(1 2 3))

;; Doall forces realization
;; TEST: doall returns seq
;; EXPECT: (1 2 3)
(doall (lazy-map identity '(1 2 3)))

;; TEST: doall side effects
;; EXPECT-FINAL: 3
(let [count 0]
  (doall (lazy-map (lambda [x] (set! count (+ count 1))) '(1 2 3)))
  count)
