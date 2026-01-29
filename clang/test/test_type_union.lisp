;; test_type_union.lisp - Tests for union types

;; Union type accepts multiple types
;; TEST: union type annotation
;; EXPECT: 42
(define process-input [x {(| Int String)}] {String}
  (if (int? x)
    (int->str x)
    x))
(process-input 42)

;; TEST: union type string branch
;; EXPECT: "hello"
(process-input "hello")

;; TEST: union of three types
;; EXPECT: "int"
(define type-name [x {(| Int Float String)}] {String}
  (cond
    (int? x)    "int"
    (float? x)  "float"
    (string? x) "string"))
(type-name 42)

;; TEST: union float branch
;; EXPECT: "float"
(type-name 3.14)

;; TEST: union string branch
;; EXPECT: "string"
(type-name "hello")

;; Nullable as union with Nil
;; TEST: nullable type
;; EXPECT: 0
(define safe-length [s {(| String Nil)}] {Int}
  (if (nil? s) 0 (str-len s)))
(safe-length nil)

;; TEST: nullable with value
;; EXPECT: 5
(safe-length "hello")

;; Result type as union
;; TEST: result type ok
;; EXPECT: #{"ok" 42}
(define divide [a {Int}] [b {Int}] {(| #{"ok" Int} #{"err" String})}
  (if (= b 0)
    #{"err" "division by zero"}
    #{"ok" (/ a b)}))
(divide 84 2)

;; TEST: result type err
;; EXPECT-FINAL: #{"err" "division by zero"}
(divide 42 0)
