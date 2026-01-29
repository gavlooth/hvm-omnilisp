;; test_pattern_wildcard.lisp - Tests for wildcard pattern matching

;; Wildcard _ matches anything but doesn't bind
(define classify [x]
  (match x
    0  "zero"
    1  "one"
    _  "other"))

;; TEST: match literal 0
;; EXPECT: "zero"
(classify 0)

;; TEST: match literal 1
;; EXPECT: "one"
(classify 1)

;; TEST: wildcard matches anything else
;; EXPECT: "other"
(classify 42)

;; TEST: wildcard matches negative
;; EXPECT: "other"
(classify -5)

;; Wildcard in list patterns
(define first-only [lst]
  (match lst
    ()       nothing
    (h .. _) h))

;; TEST: get first, ignore rest with wildcard
;; EXPECT: 1
(first-only '(1 2 3 4 5))

;; Multiple wildcards
(define second-elem [lst]
  (match lst
    ()           nothing
    (_ second .. _)  second))

;; TEST: get second element using wildcards
;; EXPECT-FINAL: 2
(second-elem '(1 2 3 4 5))
