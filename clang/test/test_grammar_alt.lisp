;; test_grammar_alt.lisp - Tests for alternative combinator

;; Alt tries alternatives in order
;; TEST: alt first matches
;; EXPECT: "cat"
(defgrammar animal
  (alt (lit "cat") (lit "dog") (lit "bird")))
(parse animal "cat")

;; TEST: alt second matches
;; EXPECT: "dog"
(parse animal "dog")

;; TEST: alt third matches
;; EXPECT: "bird"
(parse animal "bird")

;; TEST: alt none match
;; EXPECT: nil
(parse animal "fish")

;; TEST: alt with char classes
;; EXPECT: "a"
(defgrammar letter-or-digit
  (alt (char-range "a" "z") (char-range "0" "9")))
(parse letter-or-digit "a")

;; TEST: alt digit branch
;; EXPECT: "5"
(parse letter-or-digit "5")

;; Ordered choice (first match wins)
;; TEST: ordered choice longer first
;; EXPECT: "hello"
(defgrammar ordered
  (alt (lit "hello") (lit "hel")))
(parse ordered "hello")

;; TEST: ordered choice shorter first
;; EXPECT: "hel"
(defgrammar ordered2
  (alt (lit "hel") (lit "hello")))
(parse ordered2 "hello")

;; Nested alternatives
;; TEST: nested alt
;; EXPECT: "a"
(defgrammar nested-alt
  (alt (alt (lit "a") (lit "b"))
       (alt (lit "c") (lit "d"))))
(parse nested-alt "a")

;; TEST: nested alt deep
;; EXPECT: "d"
(parse nested-alt "d")

;; Alt with sequence
;; TEST: alt of sequences
;; EXPECT-FINAL: ("a" "b")
(defgrammar ab-or-cd
  (alt (seq (lit "a") (lit "b"))
       (seq (lit "c") (lit "d"))))
(parse ab-or-cd "ab")
