;; test_unquote.lisp - Tests for unquote operator

;; Unquote evaluates and inserts
;; TEST: unquote number
;; EXPECT: (answer 42)
`(answer ,(+ 40 2))

;; TEST: unquote string
;; EXPECT: (greeting "HELLO")
`(greeting ,(str-upper "hello"))

;; TEST: unquote variable
;; EXPECT: (value 100)
(let [v 100]
  `(value ,v))

;; TEST: unquote function result
;; EXPECT: (length 3)
`(length ,(length '(a b c)))

;; TEST: unquote in nested position
;; EXPECT: ((inner 5))
(let [n 5]
  `((inner ,n)))

;; TEST: unquote multiple times
;; EXPECT: (1 1 1)
(let [x 1]
  `(,x ,x ,x))

;; TEST: unquote different vars
;; EXPECT: (a 1 b 2)
(let [a 1] [b 2]
  `(a ,a b ,b))

;; Unquote preserves types
;; TEST: unquote list
;; EXPECT: (items (1 2 3))
`(items ,'(1 2 3))

;; TEST: unquote dict
;; EXPECT: (config #{"a" 1})
`(config ,#{"a" 1})

;; Unquote with conditionals
;; TEST: unquote conditional
;; EXPECT: (result positive)
(let [x 5]
  `(result ,(if (> x 0) 'positive 'negative)))

;; TEST: unquote computed symbol
;; EXPECT-FINAL: (var foo-bar)
(let [prefix "foo"]
  `(var ,(string->symbol (str-concat prefix "-bar"))))
