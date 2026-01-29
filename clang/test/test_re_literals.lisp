;; test_re_literals.lisp - Tests for regex literal characters

;; Literal characters match exactly
;; TEST: simple literal
;; EXPECT: true
(re-test #"abc" "abc")

;; TEST: literal not match
;; EXPECT: false
(re-test #"abc" "abd")

;; TEST: case sensitive
;; EXPECT: false
(re-test #"abc" "ABC")

;; TEST: case insensitive flag
;; EXPECT: true
(re-test #"(?i)abc" "ABC")

;; TEST: escaped special char
;; EXPECT: true
(re-test #"\." ".")

;; TEST: escaped backslash
;; EXPECT: true
(re-test #"\\" "\\")

;; TEST: literal brackets
;; EXPECT: true
(re-test #"\[test\]" "[test]")

;; TEST: literal parens
;; EXPECT: true
(re-test #"\(hello\)" "(hello)")

;; TEST: literal plus
;; EXPECT: true
(re-test #"a\+b" "a+b")

;; TEST: literal star
;; EXPECT: true
(re-test #"a\*b" "a*b")

;; TEST: literal question
;; EXPECT: true
(re-test #"what\?" "what?")

;; TEST: unicode literal
;; EXPECT: true
(re-test #"café" "café")

;; TEST: whitespace literal
;; EXPECT: true
(re-test #"hello world" "hello world")

;; TEST: newline literal
;; EXPECT-FINAL: true
(re-test #"line1\nline2" "line1\nline2")
