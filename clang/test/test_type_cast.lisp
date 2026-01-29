;; test_type_cast.lisp - Tests for type casting/coercion

;; Int->Float
;; TEST: int to float
;; EXPECT: 42.0
(int->float 42)

;; TEST: negative int to float
;; EXPECT: -5.0
(int->float -5)

;; Float->Int (truncates)
;; TEST: float to int truncate
;; EXPECT: 3
(float->int 3.7)

;; TEST: negative float to int
;; EXPECT: -3
(float->int -3.7)

;; String conversions
;; TEST: int to string
;; EXPECT: "42"
(int->str 42)

;; TEST: string to int
;; EXPECT: 42
(str->int "42")

;; TEST: float to string
;; EXPECT: "3.14"
(float->str 3.14)

;; TEST: string to float
;; EXPECT: 3.14
(str->float "3.14")

;; Bool conversions
;; TEST: bool to int true
;; EXPECT: 1
(bool->int true)

;; TEST: bool to int false
;; EXPECT: 0
(bool->int false)

;; TEST: int to bool zero
;; EXPECT: false
(int->bool 0)

;; TEST: int to bool nonzero
;; EXPECT: true
(int->bool 42)

;; Symbol/String conversions
;; TEST: symbol to string
;; EXPECT: "hello"
(symbol->string 'hello)

;; TEST: string to symbol
;; EXPECT: hello
(string->symbol "hello")

;; List/Array conversions
;; TEST: list to array
;; EXPECT: [1 2 3]
(list->array '(1 2 3))

;; TEST: array to list
;; EXPECT: (1 2 3)
(array->list [1 2 3])

;; Safe cast with default
(define cast-or [x] [type] [default]
  (handle
    (cast x type)
    (Raise [_ resume] default)))

;; TEST: safe cast success
;; EXPECT: 42
(cast-or "42" Int 0)

;; TEST: safe cast failure
;; EXPECT-FINAL: 0
(cast-or "not-a-number" Int 0)
