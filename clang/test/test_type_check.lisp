;; test_type_check.lisp - Tests for runtime type checking

;; Type-of returns the type of a value
;; TEST: type-of int
;; EXPECT: Int
(type-of 42)

;; TEST: type-of float
;; EXPECT: Float
(type-of 3.14)

;; TEST: type-of string
;; EXPECT: String
(type-of "hello")

;; TEST: type-of bool
;; EXPECT: Bool
(type-of true)

;; TEST: type-of nil
;; EXPECT: Nil
(type-of nil)

;; TEST: type-of list
;; EXPECT: List
(type-of '(1 2 3))

;; TEST: type-of array
;; EXPECT: Array
(type-of [1 2 3])

;; TEST: type-of dict
;; EXPECT: Dict
(type-of #{"a" 1})

;; TEST: type-of function
;; EXPECT: Fn
(type-of (lambda [x] x))

;; Instance-of checks type
;; TEST: instance-of Int
;; EXPECT: true
(instance-of? 42 Int)

;; TEST: instance-of String
;; EXPECT: true
(instance-of? "hello" String)

;; TEST: instance-of wrong type
;; EXPECT: false
(instance-of? 42 String)

;; TEST: instance-of with Number supertype
;; EXPECT: true
(instance-of? 3.14 Number)

;; Assert type
(define assert-type [x] [t]
  (if (instance-of? x t)
    x
    (perform Raise (str-concat "Expected " (type->string t)))))

;; TEST: assert-type passes
;; EXPECT: 42
(handle
  (assert-type 42 Int)
  (Raise [msg resume] nil))

;; TEST: assert-type fails
;; EXPECT-FINAL: nil
(handle
  (assert-type "hello" Int)
  (Raise [msg resume] nil))
