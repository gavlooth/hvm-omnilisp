;; test_define_variadic.lisp - Tests for variadic/rest parameters

;; Function with rest parameter using .. spread
(define sum-all [first .. rest]
  (match rest
    ()       first
    (h .. t) (+ first (sum-all h t))))

;; TEST: single argument (rest is empty)
;; EXPECT: 5
(sum-all 5)

;; TEST: two arguments
;; EXPECT: 8
(sum-all 3 5)

;; TEST: multiple arguments
;; EXPECT: 15
(sum-all 1 2 3 4 5)

;; Rest in pattern matching context
(define first-and-rest [lst]
  (match lst
    ()           '(nothing nothing)
    (h)          (cons h nil)
    (h .. t)     (cons h t)))

;; TEST: extract first and rest from list
;; EXPECT-FINAL: (1 2 3)
(first-and-rest '(1 2 3))
