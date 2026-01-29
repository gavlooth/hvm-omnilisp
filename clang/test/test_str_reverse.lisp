;; test_str_reverse.lisp - Tests for string reversal

;; Str-reverse reverses a string
;; TEST: reverse basic
;; EXPECT: "olleh"
(str-reverse "hello")

;; TEST: reverse single char
;; EXPECT: "a"
(str-reverse "a")

;; TEST: reverse empty
;; EXPECT: ""
(str-reverse "")

;; TEST: reverse palindrome
;; EXPECT: "radar"
(str-reverse "radar")

;; TEST: reverse with spaces
;; EXPECT: "dlrow olleh"
(str-reverse "hello world")

;; TEST: reverse numbers
;; EXPECT: "54321"
(str-reverse "12345")

;; Palindrome check
(define palindrome? [s]
  (= s (str-reverse s)))

;; TEST: palindrome true
;; EXPECT: true
(palindrome? "radar")

;; TEST: palindrome false
;; EXPECT: false
(palindrome? "hello")

;; TEST: palindrome empty
;; EXPECT: true
(palindrome? "")

;; TEST: palindrome single
;; EXPECT: true
(palindrome? "a")

;; TEST: palindrome even
;; EXPECT: true
(palindrome? "abba")

;; TEST: palindrome not quite
;; EXPECT-FINAL: false
(palindrome? "abca")
