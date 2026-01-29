;; test_grammar_rule.lisp - Tests for grammar rule definition

;; Define simple grammar rules
;; TEST: define terminal rule
;; EXPECT: true
(defgrammar digit
  (char-range "0" "9"))
(grammar? digit)

;; TEST: define literal rule
;; EXPECT: true
(defgrammar hello
  (lit "hello"))
(grammar? hello)

;; TEST: parse with terminal
;; EXPECT: "5"
(parse digit "5")

;; TEST: parse with literal
;; EXPECT: "hello"
(parse hello "hello")

;; TEST: parse fail
;; EXPECT: nil
(parse digit "a")

;; Define rule with reference
;; TEST: rule references another
;; EXPECT: true
(defgrammar letter
  (char-range "a" "z"))
(defgrammar word
  (one-or-more letter))
(grammar? word)

;; TEST: parse word
;; EXPECT: "hello"
(parse word "hello")

;; TEST: parse word partial
;; EXPECT: "abc"
(parse word "abc123")

;; Named rules
;; TEST: named rule
;; EXPECT: #{"type" "number" "value" "42"}
(defgrammar number
  (named "number" (one-or-more digit)))
(parse number "42")

;; TEST: grammar composition
;; EXPECT-FINAL: true
(defgrammar identifier
  (seq letter (zero-or-more (alt letter digit))))
(grammar? identifier)
