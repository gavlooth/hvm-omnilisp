;; test_syntax_pattern.lisp - Tests for syntax pattern matching

;; Syntax patterns in macro definitions
;; TEST: basic pattern variable
;; EXPECT: (+ 1 2)
(do
  (define-syntax my-add
    [(_ a b) `(+ ,a ,b)])
  (my-add 1 2))

;; TEST: pattern with rest
;; EXPECT: (list 1 2 3)
(do
  (define-syntax my-list
    [(_ items ...) `(list ,@items)])
  (my-list 1 2 3))

;; TEST: nested pattern
;; EXPECT: 3
(do
  (define-syntax swap-apply
    [(_ (f a b)) `(,f ,b ,a)])
  (swap-apply (- 5 2)))

;; TEST: pattern guards
;; EXPECT: :number
(do
  (define-syntax classify
    [(_ x) & (number? x) :number]
    [(_ x) :other])
  (classify 5))

;; TEST: multiple patterns
;; EXPECT: (2 3)
(do
  (define-syntax my-cond
    [(_ test then) `(if ,test ,then nil)]
    [(_ test then else) `(if ,test ,then ,else)])
  (list (my-cond true 2 3) (my-cond false 2 3)))

;; TEST: pattern literal matching
;; EXPECT: :matched
(do
  (define-syntax when-zero
    [(_ 0 body) body]
    [(_ _ body) nil])
  (when-zero 0 :matched))

;; TEST: pattern with keywords
;; EXPECT: 6
(do
  (define-syntax for-range
    [(_ var :from start :to end body)
     `(let loop [,var ,start]
        (if (<= ,var ,end)
          (do ,body (loop (+ ,var 1)))
          nil))])
  (let [sum 0]
    (for-range i :from 1 :to 3 (set! sum (+ sum i)))
    sum))

;; TEST: destructuring pattern
;; EXPECT-FINAL: 3
(do
  (define-syntax let-pair
    [(_ [a b] val body) `(let [a (first ,val)] (let [b (second ,val)] ,body))])
  (let-pair [x y] '(1 2) (+ x y)))
