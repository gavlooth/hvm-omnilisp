;; test_for_each.lisp - Tests for for-each (side-effecting iteration)

;; For-each applies function for side effects, returns nil
(define for-each [f] [lst]
  (match lst
    ()       nil
    (h .. t) (do (f h) (for-each f t))))

;; TEST: for-each returns nil
;; EXPECT: nil
(for-each (lambda [x] x) '(1 2 3))

;; TEST: for-each processes all elements
;; EXPECT: 6
(let [sum 0]
  (for-each (lambda [x] (set! sum (+ sum x))) '(1 2 3))
  sum)

;; TEST: for-each empty
;; EXPECT: nil
(for-each (lambda [x] (print x)) '())

;; TEST: for-each with print (side effect)
;; EXPECT: nil
(for-each println '("a" "b" "c"))

;; For-each-indexed
(define for-each-indexed [f] [lst]
  (let helper [l lst] [i 0]
    (match l
      ()       nil
      (h .. t) (do (f i h) (helper t (+ i 1))))))

;; TEST: for-each-indexed
;; EXPECT: nil
(for-each-indexed (lambda [i x] (print i x)) '(a b c))

;; Do-list macro style
(define do-list [var lst body]
  (for-each body lst))

;; TEST: do-list
;; EXPECT: nil
(do-list x '(1 2 3) (lambda [x] (print x)))

;; For-each with early exit
(define for-each-while [f] [pred] [lst]
  (match lst
    ()       nil
    (h .. t) (if (pred h)
               (do (f h) (for-each-while f pred t))
               nil)))

;; TEST: for-each-while stops
;; EXPECT: 3
(let [count 0]
  (for-each-while
    (lambda [x] (set! count (+ count 1)))
    (lambda [x] (< x 4))
    '(1 2 3 4 5))
  count)

;; TEST: for-each on dict values
;; EXPECT-FINAL: nil
(for-each println (dict-vals #{"a" 1 "b" 2}))
