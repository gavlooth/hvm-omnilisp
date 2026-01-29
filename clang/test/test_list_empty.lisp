;; test_list_empty.lisp - Tests for empty?, null?

;; Empty? checks if list is empty
(define empty? [lst]
  (match lst
    ()  true
    _   false))

;; TEST: empty list is empty
;; EXPECT: true
(empty? '())

;; TEST: non-empty list is not empty
;; EXPECT: false
(empty? '(1 2 3))

;; TEST: single element is not empty
;; EXPECT: false
(empty? '(42))

;; Null? is alias for empty?
(define null? [lst]
  (match lst
    ()  true
    _   false))

;; TEST: null on empty
;; EXPECT: true
(null? '())

;; TEST: null on non-empty
;; EXPECT: false
(null? '(1))

;; Non-empty? is opposite
(define non-empty? [lst] (not (empty? lst)))

;; TEST: non-empty on empty
;; EXPECT: false
(non-empty? '())

;; TEST: non-empty on list
;; EXPECT: true
(non-empty? '(1 2 3))

;; Safe operations using empty?
(define safe-head [lst]
  (if (empty? lst) nothing (head lst)))

;; TEST: safe-head on empty
;; EXPECT: nothing
(safe-head '())

;; TEST: safe-head on non-empty
;; EXPECT-FINAL: 1
(safe-head '(1 2 3))
