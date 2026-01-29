;; test_str_trim.lisp - Tests for string trimming

;; Str-trim removes whitespace from both ends
;; TEST: trim both sides
;; EXPECT: "hello"
(str-trim "  hello  ")

;; TEST: trim left only
;; EXPECT: "hello"
(str-trim "  hello")

;; TEST: trim right only
;; EXPECT: "hello"
(str-trim "hello  ")

;; TEST: trim nothing needed
;; EXPECT: "hello"
(str-trim "hello")

;; TEST: trim all whitespace
;; EXPECT: ""
(str-trim "   ")

;; TEST: trim empty
;; EXPECT: ""
(str-trim "")

;; TEST: trim preserves middle spaces
;; EXPECT: "hello world"
(str-trim "  hello world  ")

;; TEST: trim tabs and newlines
;; EXPECT: "hello"
(str-trim "\t\nhello\n\t")

;; Str-trim-left removes only leading whitespace
;; TEST: trim-left
;; EXPECT: "hello  "
(str-trim-left "  hello  ")

;; TEST: trim-left nothing
;; EXPECT: "hello"
(str-trim-left "hello")

;; Str-trim-right removes only trailing whitespace
;; TEST: trim-right
;; EXPECT: "  hello"
(str-trim-right "  hello  ")

;; TEST: trim-right nothing
;; EXPECT-FINAL: "hello"
(str-trim-right "hello")
