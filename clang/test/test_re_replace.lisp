;; test_re_replace.lisp - Tests for regex replace

;; Re-replace replaces first match
;; TEST: replace first
;; EXPECT: "hi world"
(re-replace "hello world" "hello" "hi")

;; TEST: replace pattern
;; EXPECT: "X23"
(re-replace "123" "[0-9]" "X")

;; TEST: replace no match
;; EXPECT: "hello"
(re-replace "hello" "[0-9]" "X")

;; TEST: replace with empty
;; EXPECT: " world"
(re-replace "hello world" "hello" "")

;; TEST: replace with captured group
;; EXPECT: "world hello"
(re-replace "hello world" "(\\w+) (\\w+)" "$2 $1")

;; TEST: replace with backreference
;; EXPECT: "hello-hello"
(re-replace "hello" "(\\w+)" "$1-$1")

;; Replace all
;; TEST: replace all
;; EXPECT: "XXX"
(re-replace-all "123" "[0-9]" "X")

;; TEST: replace all words
;; EXPECT: "X X X"
(re-replace-all "one two three" "\\w+" "X")

;; TEST: replace all with pattern
;; EXPECT: "X-X-X"
(re-replace-all "a1b2c3" "[a-z]" "X")

;; TEST: replace all none
;; EXPECT: "hello"
(re-replace-all "hello" "[0-9]" "X")

;; TEST: replace all with groups
;; EXPECT: "1a 2b 3c"
(re-replace-all "a1 b2 c3" "([a-z])([0-9])" "$2$1")

;; TEST: replace all special chars
;; EXPECT: "hello-world"
(re-replace-all "hello world" "\\s" "-")

;; TEST: replace all empty
;; EXPECT-FINAL: "hello"
(re-replace-all "hello" "x" "y")
