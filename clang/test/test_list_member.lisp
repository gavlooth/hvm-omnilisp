;; test_list_member.lisp - Tests for member?, contains?

;; Member? checks if element is in list
(define member? [x] [lst]
  (match lst
    ()       false
    (h .. t) (if (= h x) true (member? x t))))

;; TEST: member found
;; EXPECT: true
(member? 3 '(1 2 3 4 5))

;; TEST: member not found
;; EXPECT: false
(member? 10 '(1 2 3 4 5))

;; TEST: member in empty
;; EXPECT: false
(member? 1 '())

;; TEST: member first element
;; EXPECT: true
(member? 1 '(1 2 3 4 5))

;; TEST: member last element
;; EXPECT: true
(member? 5 '(1 2 3 4 5))

;; Contains? is alias for member?
(define contains? [lst] [x] (member? x lst))

;; TEST: contains found
;; EXPECT: true
(contains? '(1 2 3 4 5) 3)

;; TEST: contains not found
;; EXPECT: false
(contains? '(1 2 3 4 5) 10)

;; Count occurrences
(define count-of [x] [lst]
  (match lst
    ()       0
    (h .. t) (if (= h x)
               (+ 1 (count-of x t))
               (count-of x t))))

;; TEST: count occurrences
;; EXPECT: 3
(count-of 2 '(1 2 2 3 2 4))

;; TEST: count zero occurrences
;; EXPECT-FINAL: 0
(count-of 5 '(1 2 3 4))
