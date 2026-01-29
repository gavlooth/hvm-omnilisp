;; test_math_minmax.lisp - Tests for min and max functions

;; Basic min
;; TEST: min of two positives
;; EXPECT: 3
(min 3 7)

;; TEST: min with negative
;; EXPECT: -5
(min 3 -5)

;; TEST: min equal values
;; EXPECT: 4
(min 4 4)

;; Basic max
;; TEST: max of two positives
;; EXPECT: 7
(max 3 7)

;; TEST: max with negative
;; EXPECT: 3
(max 3 -5)

;; TEST: max equal values
;; EXPECT: 4
(max 4 4)

;; Chained min/max
;; TEST: nested min
;; EXPECT: 1
(min 5 (min 3 (min 7 1)))

;; TEST: nested max
;; EXPECT: 9
(max 5 (max 3 (max 9 1)))

;; Clamp function using min/max
(define clamp [x lo hi]
  (min hi (max lo x)))

;; TEST: clamp within range
;; EXPECT: 5
(clamp 5 0 10)

;; TEST: clamp below min
;; EXPECT: 0
(clamp -5 0 10)

;; TEST: clamp above max
;; EXPECT-FINAL: 10
(clamp 15 0 10)
