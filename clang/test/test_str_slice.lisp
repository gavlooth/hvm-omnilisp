;; test_str_slice.lisp - Tests for string slicing

;; Str-slice extracts a substring
;; TEST: slice middle
;; EXPECT: "ell"
(str-slice "hello" 1 4)

;; TEST: slice from start
;; EXPECT: "hel"
(str-slice "hello" 0 3)

;; TEST: slice to end
;; EXPECT: "llo"
(str-slice "hello" 2 5)

;; TEST: slice single char
;; EXPECT: "e"
(str-slice "hello" 1 2)

;; TEST: slice whole string
;; EXPECT: "hello"
(str-slice "hello" 0 5)

;; TEST: slice empty range
;; EXPECT: ""
(str-slice "hello" 2 2)

;; Substring is alias
(define substring [s] [start] [end]
  (str-slice s start end))

;; TEST: substring
;; EXPECT: "orl"
(substring "world" 1 4)

;; Take first n chars
(define str-take [s] [n]
  (str-slice s 0 n))

;; TEST: str-take
;; EXPECT: "hel"
(str-take "hello" 3)

;; Drop first n chars
(define str-drop [s] [n]
  (str-slice s n (str-len s)))

;; TEST: str-drop
;; EXPECT: "lo"
(str-drop "hello" 3)

;; TEST: str-take whole
;; EXPECT: "hello"
(str-take "hello" 10)

;; TEST: str-drop all
;; EXPECT-FINAL: ""
(str-drop "hello" 10)
