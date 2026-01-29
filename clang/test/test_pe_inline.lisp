;; test_pe_inline.lisp - Tests for partial evaluation inlining

;; Partial evaluator inlines function calls
;; TEST: inline simple function
;; EXPECT: (+ x 2)
(pe-inline
  '(let [add2 (lambda [x] (+ x 2))]
     (add2 x)))

;; TEST: inline removes lambda
;; EXPECT: (* 3 3)
(pe-inline
  '((lambda [x] (* x x)) 3))

;; TEST: inline nested calls
;; EXPECT: (+ (+ x 1) 1)
(pe-inline
  '(let [inc (lambda [n] (+ n 1))]
     (inc (inc x))))

;; TEST: inline preserves free vars
;; EXPECT: (+ x y)
(pe-inline
  '(let [add (lambda [a b] (+ a b))]
     (add x y)))

;; TEST: no inline when recursive
;; EXPECT: (fact (- n 1))
(pe-inline
  '(let [fact (lambda [n]
                (if (= n 0) 1 (* n (fact (- n 1)))))]
     (fact 5))
  :inline-recursive false)

;; TEST: inline with ^:inline hint
;; EXPECT: 10
(do
  (define ^:inline double [x] (* x 2))
  (pe-eval '(double 5)))

;; TEST: inline threshold
;; EXPECT: true
(let [code (pe-inline '((lambda [x] (+ x x x x x)) n)
                      :max-inline-size 10)]
  (not (list? (first code))))

;; TEST: inline respects noinline
;; EXPECT: (heavy-computation x)
(pe-inline
  '(let [^:noinline heavy-computation (lambda [x] (expensive x))]
     (heavy-computation x)))

;; TEST: beta reduction
;; EXPECT-FINAL: 6
(pe-eval
  '((lambda [x y] (+ x y)) 2 4))
