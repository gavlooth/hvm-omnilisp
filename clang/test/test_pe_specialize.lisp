;; test_pe_specialize.lisp - Tests for function specialization

;; Specialize creates optimized function for known arguments
;; TEST: specialize power
;; EXPECT: true
(define power [base] [exp]
  (if (= exp 0)
    1
    (* base (power base (- exp 1)))))
(fn? (specialize power _ 3))

;; TEST: specialized version works
;; EXPECT: 8
(let [cube (specialize power _ 3)]
  (cube 2))

;; TEST: specialization unfolds
;; EXPECT: '(* base (* base (* base 1)))
(pe-specialize 'power '(_ 3))

;; TEST: specialize first arg
;; EXPECT: 16
(let [powers-of-2 (specialize power 2 _)]
  (powers-of-2 4))

;; Specialize with multiple args
;; TEST: specialize add
;; EXPECT: 15
(define add3 [a] [b] [c] (+ a (+ b c)))
(let [add-10-to (specialize add3 10 _ _)]
  (add-10-to 2 3))

;; Memoized specialization
;; TEST: memo-specialize
;; EXPECT: true
(let [s1 (specialize power _ 3)]
      [s2 (specialize power _ 3)]
  (= s1 s2))

;; Specialize recursive
;; TEST: specialize fib for known n
;; EXPECT: 55
(define fib [n]
  (if (<= n 1) n (+ (fib (- n 1)) (fib (- n 2)))))
(let [fib-10 (specialize-full fib 10)]
  (fib-10))

;; Residual code
;; TEST: residual after specialization
;; EXPECT-FINAL: '(lambda [base] (* base (* base (* base 1))))
(pe-residual 'power '(_ 3))
