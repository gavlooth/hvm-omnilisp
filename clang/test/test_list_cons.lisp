;; test_list_cons.lisp - Tests for cons, car, cdr

;; Basic cons
;; TEST: cons creates a pair
;; EXPECT: (1 2 3)
(cons 1 '(2 3))

;; TEST: cons with nil
;; EXPECT: (1)
(cons 1 nil)

;; TEST: nested cons
;; EXPECT: (1 2 3)
(cons 1 (cons 2 (cons 3 nil)))

;; Car (head)
;; TEST: car of list
;; EXPECT: 1
(car '(1 2 3))

;; TEST: car of single element
;; EXPECT: 42
(car '(42))

;; Cdr (tail)
;; TEST: cdr of list
;; EXPECT: (2 3)
(cdr '(1 2 3))

;; TEST: cdr of single element
;; EXPECT: ()
(cdr '(42))

;; Combined car/cdr
;; TEST: car of cdr (second element)
;; EXPECT: 2
(car (cdr '(1 2 3)))

;; TEST: cdr of cdr
;; EXPECT: (3)
(cdr (cdr '(1 2 3)))

;; TEST: car of cdr of cdr (third element)
;; EXPECT-FINAL: 3
(car (cdr (cdr '(1 2 3))))
