;; test_cycle.lisp - Tests for cycle (infinite repetition)

;; Cycle repeats sequence infinitely
;; TEST: cycle creates lazy seq
;; EXPECT: true
(lazy-seq? (cycle '(1 2 3)))

;; TEST: take from cycle
;; EXPECT: (1 2 3 1 2 3 1 2)
(take 8 (cycle '(1 2 3)))

;; TEST: cycle single element
;; EXPECT: (a a a a a)
(take 5 (cycle '(a)))

;; TEST: cycle two elements
;; EXPECT: (0 1 0 1 0 1)
(take 6 (cycle '(0 1)))

;; TEST: first of cycle
;; EXPECT: 1
(first (cycle '(1 2 3)))

;; TEST: second of cycle
;; EXPECT: 2
(second (cycle '(1 2 3)))

;; Cycle with transformation
;; TEST: map over cycle
;; EXPECT: (2 4 6 2 4 6)
(take 6 (lazy-map (lambda [x] (* x 2)) (cycle '(1 2 3))))

;; TEST: filter cycle
;; EXPECT: (2 2 2 2 2)
(take 5 (lazy-filter (lambda [x] (= (% x 2) 0)) (cycle '(1 2 3))))

;; Use cycle for round-robin
;; TEST: zip with cycle
;; EXPECT: ((a 1) (b 2) (c 1) (d 2))
(take 4 (lazy-map list '(a b c d e f) (cycle '(1 2))))

;; Interleave (related to cycle)
;; TEST: interleave
;; EXPECT: (1 a 2 b 3 c)
(take 6 (interleave '(1 2 3 4 5) '(a b c d e)))

;; TEST: cycle empty (returns empty)
;; EXPECT-FINAL: ()
(take 5 (cycle '()))
