;; test_compare.lisp - Tests for compare function (returns -1, 0, 1)

;; Compare numbers
;; TEST: compare less than
;; EXPECT: -1
(compare 3 5)

;; TEST: compare greater than
;; EXPECT: 1
(compare 5 3)

;; TEST: compare equal
;; EXPECT: 0
(compare 5 5)

;; Compare with negatives
;; TEST: negative vs positive
;; EXPECT: -1
(compare -5 5)

;; TEST: positive vs negative
;; EXPECT: 1
(compare 5 -5)

;; Use compare for sorting predicate
(define compare-ascending [a] [b]
  (< (compare a b) 0))

;; TEST: ascending comparison true
;; EXPECT: true
(compare-ascending 3 5)

;; TEST: ascending comparison false
;; EXPECT: false
(compare-ascending 5 3)

;; Spaceship comparison helper
(define three-way [a] [b]
  (match (compare a b)
    -1  "less"
    0   "equal"
    1   "greater"))

;; TEST: three-way less
;; EXPECT: "less"
(three-way 1 5)

;; TEST: three-way equal
;; EXPECT: "equal"
(three-way 5 5)

;; TEST: three-way greater
;; EXPECT-FINAL: "greater"
(three-way 9 5)
