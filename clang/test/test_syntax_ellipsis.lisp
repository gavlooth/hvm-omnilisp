;; test_syntax_ellipsis.lisp - Tests for ellipsis patterns in macros

;; Ellipsis matches zero or more
;; TEST: ellipsis zero elements
;; EXPECT: ()
(define-syntax my-list
  (syntax-rules ()
    ((my-list x ...) '(x ...))))
(my-list)

;; TEST: ellipsis one element
;; EXPECT: (1)
(my-list 1)

;; TEST: ellipsis many elements
;; EXPECT: (1 2 3 4 5)
(my-list 1 2 3 4 5)

;; Ellipsis in body
;; TEST: begin-style macro
;; EXPECT: 3
(define-syntax my-begin
  (syntax-rules ()
    ((my-begin expr ...)
     (do expr ...))))
(my-begin 1 2 3)

;; TEST: and-style macro
;; EXPECT: true
(define-syntax my-and
  (syntax-rules ()
    ((my-and) true)
    ((my-and x) x)
    ((my-and x y ...) (if x (my-and y ...) false))))
(my-and true true true)

;; TEST: and short circuit
;; EXPECT: false
(my-and true false true)

;; TEST: or-style macro
;; EXPECT: true
(define-syntax my-or
  (syntax-rules ()
    ((my-or) false)
    ((my-or x) x)
    ((my-or x y ...) (if x true (my-or y ...)))))
(my-or false false true)

;; Nested ellipsis
;; TEST: nested ellipsis
;; EXPECT: ((1 2) (3 4))
(define-syntax group-pairs
  (syntax-rules ()
    ((group-pairs (a b) ...) '((a b) ...))))
(group-pairs (1 2) (3 4))

;; TEST: ellipsis preserves order
;; EXPECT-FINAL: (5 4 3 2 1)
(my-list 5 4 3 2 1)
