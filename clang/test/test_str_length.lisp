;; test_str_length.lisp - Tests for string length

;; TEST: length of normal string
;; EXPECT: 5
(str-len "hello")

;; TEST: length of empty string
;; EXPECT: 0
(str-len "")

;; TEST: length of single char
;; EXPECT: 1
(str-len "a")

;; TEST: length with spaces
;; EXPECT: 11
(str-len "hello world")

;; TEST: length with numbers
;; EXPECT: 5
(str-len "12345")

;; TEST: length of longer string
;; EXPECT: 26
(str-len "abcdefghijklmnopqrstuvwxyz")

;; String empty predicate
(define str-empty? [s]
  (= (str-len s) 0))

;; TEST: str-empty? true
;; EXPECT: true
(str-empty? "")

;; TEST: str-empty? false
;; EXPECT: false
(str-empty? "x")

;; String not empty predicate
(define str-not-empty? [s]
  (> (str-len s) 0))

;; TEST: str-not-empty? with content
;; EXPECT: true
(str-not-empty? "hello")

;; TEST: str-not-empty? empty
;; EXPECT-FINAL: false
(str-not-empty? "")
