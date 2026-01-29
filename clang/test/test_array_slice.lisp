;; test_array_slice.lisp - Tests for array slicing

;; Slice extracts sub-array from start to end (exclusive)
;; TEST: slice middle portion
;; EXPECT: [3 4 5]
(arr-slice [1 2 3 4 5 6 7] 2 5)

;; TEST: slice from beginning
;; EXPECT: [1 2 3]
(arr-slice [1 2 3 4 5] 0 3)

;; TEST: slice to end
;; EXPECT: [4 5]
(arr-slice [1 2 3 4 5] 3 5)

;; TEST: slice single element
;; EXPECT: [3]
(arr-slice [1 2 3 4 5] 2 3)

;; TEST: slice empty (start = end)
;; EXPECT: []
(arr-slice [1 2 3 4 5] 2 2)

;; TEST: slice entire array
;; EXPECT: [1 2 3 4 5]
(arr-slice [1 2 3 4 5] 0 5)

;; Helper: take first n from array
(define arr-take [arr] [n]
  (arr-slice arr 0 n))

;; TEST: arr-take
;; EXPECT: [1 2 3]
(arr-take [1 2 3 4 5] 3)

;; Helper: drop first n from array
(define arr-drop [arr] [n]
  (arr-slice arr n (arr-len arr)))

;; TEST: arr-drop
;; EXPECT: [4 5]
(arr-drop [1 2 3 4 5] 3)

;; Split array at position
(define arr-split-at [arr] [n]
  (cons (arr-take arr n) (cons (arr-drop arr n) nil)))

;; TEST: split array at position
;; EXPECT-FINAL: ([1 2] [3 4 5])
(arr-split-at [1 2 3 4 5] 2)
