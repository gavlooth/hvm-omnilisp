;; test_list_unique.lisp - Tests for removing duplicates

;; Unique removes duplicate elements
;; TEST: basic unique
;; EXPECT: (1 2 3)
(unique '(1 2 2 3 3 3))

;; TEST: nub (alias)
;; EXPECT: (a b c)
(nub '(a b a c b c))

;; TEST: distinct (alias)
;; EXPECT: (1 2 3)
(distinct '(1 1 2 2 3 3))

;; TEST: unique empty
;; EXPECT: ()
(unique '())

;; TEST: unique single
;; EXPECT: (1)
(unique '(1))

;; TEST: unique all same
;; EXPECT: (x)
(unique '(x x x x x))

;; TEST: unique preserves order
;; EXPECT: (3 1 2)
(unique '(3 1 2 1 3 2))

;; TEST: unique-by key function
;; EXPECT: ((1 "a") (2 "b"))
(unique-by first '((1 "a") (1 "b") (2 "b")))

;; TEST: dedupe consecutive
;; EXPECT: (1 2 3 2 1)
(dedupe '(1 1 2 2 3 3 2 2 1))

;; TEST: dedupe-by consecutive
;; EXPECT: ("a" "bb" "c")
(dedupe-by str-length '("a" "x" "bb" "yy" "c"))

;; TEST: frequencies count
;; EXPECT: #{"a" 3 "b" 2 "c" 1}
(frequencies '(a a a b b c))

;; TEST: unique with nil
;; EXPECT-FINAL: (nil 1 2)
(unique '(nil 1 nil 2 nil 1))
