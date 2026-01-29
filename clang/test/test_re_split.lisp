;; test_re_split.lisp - Tests for regex split

;; Re-split splits string by pattern
;; TEST: split by space
;; EXPECT: ("hello" "world")
(re-split "hello world" "\\s+")

;; TEST: split by comma
;; EXPECT: ("a" "b" "c")
(re-split "a,b,c" ",")

;; TEST: split by multiple spaces
;; EXPECT: ("one" "two" "three")
(re-split "one   two   three" "\\s+")

;; TEST: split by digit
;; EXPECT: ("a" "b" "c" "d")
(re-split "a1b2c3d" "[0-9]")

;; TEST: split no match
;; EXPECT: ("hello world")
(re-split "hello world" ",")

;; TEST: split at start
;; EXPECT: ("" "hello")
(re-split ",hello" ",")

;; TEST: split at end
;; EXPECT: ("hello" "")
(re-split "hello," ",")

;; TEST: split multiple patterns
;; EXPECT: ("a" "b" "c" "d")
(re-split "a,b;c:d" "[,;:]")

;; TEST: split empty parts
;; EXPECT: ("a" "" "b")
(re-split "a,,b" ",")

;; TEST: split by word boundary
;; EXPECT: ("hello" " " "world")
(re-split "hello world" "(?<=\\w)(?=\\s)|(?<=\\s)(?=\\w)")

;; TEST: split limit
;; EXPECT: ("a" "b,c,d")
(re-split "a,b,c,d" "," 2)

;; TEST: split empty string
;; EXPECT: ()
(re-split "" ",")

;; TEST: split into chars
;; EXPECT-FINAL: ("h" "e" "l" "l" "o")
(re-split "hello" "")
