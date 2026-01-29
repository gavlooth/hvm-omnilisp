;; test_define_syntax.lisp - Tests for syntax macro definition

;; Define a simple syntax macro
;; TEST: define and use macro
;; EXPECT: 3
(define-syntax my-add
  (syntax-rules ()
    ((my-add a b) (+ a b))))
(my-add 1 2)

;; TEST: macro with single form
;; EXPECT: 42
(define-syntax identity-macro
  (syntax-rules ()
    ((identity-macro x) x)))
(identity-macro 42)

;; TEST: macro with multiple patterns
;; EXPECT: 1
(define-syntax my-if
  (syntax-rules ()
    ((my-if test then) (if test then nil))
    ((my-if test then else) (if test then else))))
(my-if true 1)

;; TEST: macro with else
;; EXPECT: 2
(my-if false 1 2)

;; TEST: let-style macro
;; EXPECT: 30
(define-syntax my-let
  (syntax-rules ()
    ((my-let ((var val)) body)
     (let [var val] body))))
(my-let ((x 10)) (* x 3))

;; TEST: when macro
;; EXPECT: "yes"
(define-syntax my-when
  (syntax-rules ()
    ((my-when test body ...)
     (if test (do body ...) nil))))
(my-when true "yes")

;; TEST: unless macro
;; EXPECT: nil
(define-syntax my-unless
  (syntax-rules ()
    ((my-unless test body ...)
     (if test nil (do body ...)))))
(my-unless true "no")

;; TEST: unless with false
;; EXPECT-FINAL: "yes"
(my-unless false "yes")
