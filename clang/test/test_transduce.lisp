;; test_transduce.lisp - Tests for transducers

;; Transducers compose transformations efficiently
;; TEST: basic transduce
;; EXPECT: 14
(transduce (map (lambda [x] (* x 2)))
           +
           0
           '(1 2 3 4))

;; TEST: composed transducers
;; EXPECT: 12
(transduce (comp (filter odd?) (map (lambda [x] (* x 2))))
           +
           0
           '(1 2 3 4 5))

;; TEST: transduce into list
;; EXPECT: (2 4 6)
(into '()
  (map (lambda [x] (* x 2)))
  '(1 2 3))

;; TEST: transduce into vector
;; EXPECT: [2 4 6]
(into []
  (map (lambda [x] (* x 2)))
  '(1 2 3))

;; TEST: take transducer
;; EXPECT: (1 2 3)
(into '() (take 3) '(1 2 3 4 5))

;; TEST: drop transducer
;; EXPECT: (4 5)
(into '() (drop 3) '(1 2 3 4 5))

;; TEST: partition-all transducer
;; EXPECT: ((1 2) (3 4) (5))
(into '() (partition-all 2) '(1 2 3 4 5))

;; TEST: cat transducer (flatten one level)
;; EXPECT: (1 2 3 4)
(into '() cat '((1 2) (3 4)))

;; TEST: mapcat transducer
;; EXPECT: (1 1 2 2 3 3)
(into '() (mapcat (lambda [x] (list x x))) '(1 2 3))

;; TEST: early termination with take
;; EXPECT: 6
(transduce (comp (map (lambda [x] (* x 2))) (take 3))
           +
           0
           '(1 2 3 4 5 6 7 8 9 10))

;; TEST: dedupe transducer
;; EXPECT-FINAL: (1 2 3 2)
(into '() dedupe '(1 1 2 2 2 3 3 2))
