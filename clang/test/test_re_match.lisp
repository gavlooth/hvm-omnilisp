;; test_re_match.lisp - Tests for regex full matching

;; Re-match tests if entire string matches pattern
;; TEST: simple match
;; EXPECT: true
(re-match? "hello" "hello")

;; TEST: no match
;; EXPECT: false
(re-match? "hello" "world")

;; TEST: partial doesn't match
;; EXPECT: false
(re-match? "hello world" "hello")

;; TEST: wildcard match
;; EXPECT: true
(re-match? "hello" "h.llo")

;; TEST: star match
;; EXPECT: true
(re-match? "helllo" "hel*o")

;; TEST: plus match
;; EXPECT: true
(re-match? "helllo" "hel+o")

;; TEST: question match
;; EXPECT: true
(re-match? "helo" "hel?lo")

;; TEST: character class
;; EXPECT: true
(re-match? "hello" "[a-z]+")

;; TEST: digit class
;; EXPECT: true
(re-match? "12345" "[0-9]+")

;; TEST: mixed match
;; EXPECT: true
(re-match? "abc123" "[a-z]+[0-9]+")

;; TEST: anchored match (implicit in full match)
;; EXPECT: true
(re-match? "test" "^test$")

;; TEST: case sensitive
;; EXPECT: false
(re-match? "Hello" "hello")

;; TEST: empty string
;; EXPECT: true
(re-match? "" "")

;; TEST: empty pattern on non-empty
;; EXPECT-FINAL: false
(re-match? "hello" "")
