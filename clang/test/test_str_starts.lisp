;; test_str_starts.lisp - Tests for string prefix/suffix predicates

;; Str-starts? checks if string starts with prefix
;; TEST: starts with - true
;; EXPECT: true
(str-starts? "hello world" "hello")

;; TEST: starts with - false
;; EXPECT: false
(str-starts? "hello world" "world")

;; TEST: starts with empty prefix
;; EXPECT: true
(str-starts? "hello" "")

;; TEST: starts with same string
;; EXPECT: true
(str-starts? "hello" "hello")

;; TEST: starts with longer prefix
;; EXPECT: false
(str-starts? "hi" "hello")

;; TEST: starts with single char
;; EXPECT: true
(str-starts? "hello" "h")

;; Str-ends? checks if string ends with suffix
;; TEST: ends with - true
;; EXPECT: true
(str-ends? "hello world" "world")

;; TEST: ends with - false
;; EXPECT: false
(str-ends? "hello world" "hello")

;; TEST: ends with empty suffix
;; EXPECT: true
(str-ends? "hello" "")

;; TEST: ends with same string
;; EXPECT: true
(str-ends? "hello" "hello")

;; TEST: ends with longer suffix
;; EXPECT: false
(str-ends? "hi" "hello")

;; TEST: ends with single char
;; EXPECT-FINAL: true
(str-ends? "hello" "o")
