;; test_list_head_tail.lisp - Tests for head, tail, last, init

;; Head (same as car)
;; TEST: head of list
;; EXPECT: 1
(head '(1 2 3 4 5))

;; TEST: head of single element
;; EXPECT: 42
(head '(42))

;; Tail (same as cdr)
;; TEST: tail of list
;; EXPECT: (2 3 4 5)
(tail '(1 2 3 4 5))

;; TEST: tail of single element
;; EXPECT: ()
(tail '(42))

;; Last (final element)
(define last [lst]
  (match lst
    ()        nothing
    (x)       x
    (_ .. t)  (last t)))

;; TEST: last of list
;; EXPECT: 5
(last '(1 2 3 4 5))

;; TEST: last of single element
;; EXPECT: 42
(last '(42))

;; Init (all but last)
(define init [lst]
  (match lst
    ()        '()
    (x)       '()
    (h .. t)  (cons h (init t))))

;; TEST: init of list
;; EXPECT: (1 2 3 4)
(init '(1 2 3 4 5))

;; TEST: init of single element
;; EXPECT-FINAL: ()
(init '(42))
