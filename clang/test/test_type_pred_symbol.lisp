;; test_type_pred_symbol.lisp - Tests for symbol predicates

;; TEST: symbol? with quoted symbol
;; EXPECT: true
(symbol? 'hello)

;; TEST: symbol? with keyword
;; EXPECT: true
(symbol? :keyword)

;; TEST: symbol? with string
;; EXPECT: false
(symbol? "hello")

;; TEST: symbol? with int
;; EXPECT: false
(symbol? 42)

;; Keywords are symbols starting with :
(define keyword? [x]
  (and (symbol? x)
       (str-starts? (symbol->string x) ":")))

;; TEST: keyword? with keyword
;; EXPECT: true
(keyword? :key)

;; TEST: keyword? with regular symbol
;; EXPECT: false
(keyword? 'regular)

;; TEST: keyword? with string
;; EXPECT: false
(keyword? ":key")

;; Symbol name extraction
(define symbol-name [s]
  (symbol->string s))

;; TEST: symbol-name
;; EXPECT: "hello"
(symbol-name 'hello)

;; TEST: symbol-name keyword
;; EXPECT: ":key"
(symbol-name :key)

;; Create symbol from string
;; TEST: string->symbol
;; EXPECT: hello
(string->symbol "hello")

;; Gensym creates unique symbol
;; TEST: gensym creates symbol
;; EXPECT: true
(symbol? (gensym "prefix"))

;; TEST: two gensyms are different
;; EXPECT: false
(= (gensym "g") (gensym "g"))

;; Qualified symbol check
(define qualified? [s]
  (and (symbol? s)
       (str-contains? (symbol->string s) "/")))

;; TEST: qualified symbol
;; EXPECT: true
(qualified? 'namespace/name)

;; TEST: unqualified symbol
;; EXPECT-FINAL: false
(qualified? 'name)
