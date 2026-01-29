;; test_list_sort.lisp - Tests for list sorting

;; Sort orders elements
;; TEST: sort numbers ascending
;; EXPECT: (1 2 3 4 5)
(sort '(3 1 4 1 5 2))

;; TEST: sort already sorted
;; EXPECT: (1 2 3)
(sort '(1 2 3))

;; TEST: sort reverse
;; EXPECT: (1 2 3 4 5)
(sort '(5 4 3 2 1))

;; TEST: sort empty
;; EXPECT: ()
(sort '())

;; TEST: sort single
;; EXPECT: (42)
(sort '(42))

;; TEST: sort with duplicates
;; EXPECT: (1 1 2 2 3)
(sort '(2 1 2 1 3))

;; TEST: sort strings
;; EXPECT: ("a" "b" "c")
(sort '("c" "a" "b"))

;; TEST: sort-by key function
;; EXPECT: ((1 "z") (2 "y") (3 "x"))
(sort-by first '((3 "x") (1 "z") (2 "y")))

;; TEST: sort-by string length
;; EXPECT: ("a" "bb" "ccc")
(sort-by str-length '("bb" "ccc" "a"))

;; TEST: sort descending
;; EXPECT: (5 4 3 2 1)
(sort-desc '(3 1 4 5 2))

;; TEST: sort with comparator
;; EXPECT: (5 4 3 2 1)
(sort-with > '(3 1 4 5 2))

;; TEST: stable sort preserves order
;; EXPECT-FINAL: (("a" 1) ("b" 1) ("c" 2))
(stable-sort-by second '(("b" 1) ("a" 1) ("c" 2)))
