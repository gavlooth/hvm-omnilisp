;; test_str_to_int.lisp - Tests for string to integer conversion

;; Str->int parses string to integer
;; TEST: parse positive
;; EXPECT: 42
(str->int "42")

;; TEST: parse zero
;; EXPECT: 0
(str->int "0")

;; TEST: parse negative
;; EXPECT: -123
(str->int "-123")

;; TEST: parse large number
;; EXPECT: 999999
(str->int "999999")

;; TEST: parse with leading zeros
;; EXPECT: 7
(str->int "007")

;; Parse-int is alias
(define parse-int [s] (str->int s))

;; TEST: parse-int
;; EXPECT: 100
(parse-int "100")

;; Int->str converts integer to string
;; TEST: int to string positive
;; EXPECT: "42"
(int->str 42)

;; TEST: int to string zero
;; EXPECT: "0"
(int->str 0)

;; TEST: int to string negative
;; EXPECT: "-123"
(int->str -123)

;; TEST: int to string large
;; EXPECT: "999999"
(int->str 999999)

;; Roundtrip test
;; TEST: roundtrip int->str->int
;; EXPECT: 12345
(str->int (int->str 12345))

;; TEST: roundtrip str->int->str
;; EXPECT-FINAL: "6789"
(int->str (str->int "6789"))
