;; test_re_alternation.lisp - Tests for regex alternation

;; Alternation matches one of multiple patterns
;; TEST: basic alternation
;; EXPECT: true
(re-test #"cat|dog" "cat")

;; TEST: alternation second option
;; EXPECT: true
(re-test #"cat|dog" "dog")

;; TEST: alternation no match
;; EXPECT: false
(re-test #"cat|dog" "bird")

;; TEST: multiple alternatives
;; EXPECT: true
(re-test #"red|green|blue" "green")

;; TEST: alternation with groups
;; EXPECT: "gray"
(first (re-find #"gr(a|e)y" "gray"))

;; TEST: alternation both match (first wins)
;; EXPECT: "cat"
(first (re-find #"cat|catalog" "catalog"))

;; TEST: longer first to match longer
;; EXPECT: "catalog"
(first (re-find #"catalog|cat" "catalog"))

;; TEST: alternation in group
;; EXPECT: true
(re-test #"^(yes|no)$" "yes")

;; TEST: empty alternative
;; EXPECT: true
(re-test #"a(b|)c" "ac")

;; TEST: nested alternation
;; EXPECT: true
(re-test #"((a|b)|(c|d))" "c")

;; TEST: alternation with quantifier
;; EXPECT: true
(re-test #"(ab|cd)+" "abcdab")

;; TEST: find all alternatives
;; EXPECT: ("cat" "dog" "cat")
(re-find-all #"cat|dog" "I have a cat and a dog and another cat")

;; TEST: non-capturing alternation
;; EXPECT-FINAL: true
(re-test #"(?:yes|no)" "yes")
