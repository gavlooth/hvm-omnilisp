;; test_re_test.lisp - Tests for regex boolean test

;; Re-test checks if pattern exists anywhere in string
;; TEST: pattern found
;; EXPECT: true
(re-test "hello world" "world")

;; TEST: pattern not found
;; EXPECT: false
(re-test "hello world" "foo")

;; TEST: pattern at start
;; EXPECT: true
(re-test "hello world" "hello")

;; TEST: pattern in middle
;; EXPECT: true
(re-test "hello world" "lo wo")

;; TEST: pattern at end
;; EXPECT: true
(re-test "hello world" "orld")

;; TEST: wildcard search
;; EXPECT: true
(re-test "hello world" "w..ld")

;; TEST: digit search
;; EXPECT: true
(re-test "abc123def" "[0-9]+")

;; TEST: no digits
;; EXPECT: false
(re-test "abcdef" "[0-9]+")

;; TEST: word boundary
;; EXPECT: true
(re-test "hello world" "\\bworld\\b")

;; TEST: case insensitive flag
;; EXPECT: true
(re-test "Hello World" "(?i)hello")

;; TEST: empty pattern always matches
;; EXPECT: true
(re-test "anything" "")

;; TEST: empty string
;; EXPECT: true
(re-test "" "")

;; TEST: whitespace
;; EXPECT: true
(re-test "hello world" "\\s")

;; TEST: no whitespace
;; EXPECT-FINAL: false
(re-test "helloworld" "\\s")
