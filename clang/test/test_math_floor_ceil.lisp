;; test_math_floor_ceil.lisp - Tests for floor, ceil, round, truncate

;; Floor (round toward negative infinity)
;; TEST: floor of positive float
;; EXPECT: 3
(floor 3.7)

;; TEST: floor of negative float
;; EXPECT: -4
(floor -3.2)

;; TEST: floor of integer
;; EXPECT: 5
(floor 5)

;; Ceiling (round toward positive infinity)
;; TEST: ceil of positive float
;; EXPECT: 4
(ceil 3.2)

;; TEST: ceil of negative float
;; EXPECT: -3
(ceil -3.7)

;; TEST: ceil of integer
;; EXPECT: 5
(ceil 5)

;; Round (round to nearest integer)
;; TEST: round down
;; EXPECT: 3
(round 3.2)

;; TEST: round up
;; EXPECT: 4
(round 3.7)

;; TEST: round half (rounds to even typically)
;; EXPECT: 4
(round 3.5)

;; Truncate (round toward zero)
;; TEST: truncate positive
;; EXPECT: 3
(truncate 3.7)

;; TEST: truncate negative (toward zero, not floor)
;; EXPECT-FINAL: -3
(truncate -3.7)
