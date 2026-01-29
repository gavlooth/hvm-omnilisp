;; test_str_split.lisp - Tests for string splitting

;; Str-split splits string by delimiter
;; TEST: split by comma
;; EXPECT: ("a" "b" "c")
(str-split "a,b,c" ",")

;; TEST: split by space
;; EXPECT: ("hello" "world")
(str-split "hello world" " ")

;; TEST: split no delimiter present
;; EXPECT: ("hello")
(str-split "hello" ",")

;; TEST: split by longer delimiter
;; EXPECT: ("a" "b" "c")
(str-split "a--b--c" "--")

;; TEST: split empty string
;; EXPECT: ()
(str-split "" ",")

;; TEST: split with empty parts
;; EXPECT: ("a" "" "b")
(str-split "a,,b" ",")

;; TEST: split at start
;; EXPECT: ("" "a" "b")
(str-split ",a,b" ",")

;; TEST: split at end
;; EXPECT: ("a" "b" "")
(str-split "a,b," ",")

;; Lines splits by newline
(define lines [s]
  (str-split s "\n"))

;; TEST: lines
;; EXPECT: ("line1" "line2" "line3")
(lines "line1\nline2\nline3")

;; Words splits by space
(define words [s]
  (str-split s " "))

;; TEST: words
;; EXPECT-FINAL: ("hello" "world" "today")
(words "hello world today")
