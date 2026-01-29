;; test_letrec.lisp - Tests for recursive let bindings

;; Letrec allows mutually recursive bindings
;; TEST: self-recursive function
;; EXPECT: 120
(letrec [factorial (lambda [n]
                     (if (<= n 1) 1 (* n (factorial (- n 1)))))]
  (factorial 5))

;; TEST: mutual recursion
;; EXPECT: true
(letrec [is-even (lambda [n] (if (= n 0) true (is-odd (- n 1))))]
        [is-odd (lambda [n] (if (= n 0) false (is-even (- n 1))))]
  (is-even 10))

;; TEST: mutual recursion odd
;; EXPECT: true
(letrec [is-even (lambda [n] (if (= n 0) true (is-odd (- n 1))))]
        [is-odd (lambda [n] (if (= n 0) false (is-even (- n 1))))]
  (is-odd 7))

;; TEST: letrec with multiple uses
;; EXPECT: (1 1 2 3 5 8)
(letrec [fib (lambda [n]
               (if (< n 2) 1 (+ (fib (- n 1)) (fib (- n 2)))))]
  (map fib '(0 1 2 3 4 5)))

;; TEST: letrec nested
;; EXPECT: 55
(letrec [outer (lambda [n]
                 (letrec [inner (lambda [i acc]
                                  (if (> i n) acc (inner (+ i 1) (+ acc i))))]
                   (inner 1 0)))]
  (outer 10))

;; TEST: letrec with helper
;; EXPECT: (3 2 1)
(letrec [reverse-helper (lambda [lst acc]
                          (match lst
                            ()       acc
                            (h .. t) (reverse-helper t (cons h acc))))]
  (reverse-helper '(1 2 3) '()))

;; TEST: three-way recursion
;; EXPECT: 0
(letrec [a (lambda [n] (if (= n 0) 0 (b (- n 1))))]
        [b (lambda [n] (if (= n 0) 0 (c (- n 1))))]
        [c (lambda [n] (if (= n 0) 0 (a (- n 1))))]
  (a 9))

;; TEST: letrec shadow outer
;; EXPECT-FINAL: 10
(let [x 5]
  (letrec [f (lambda [n] (if (= n 0) x (f (- n 1))))]
    (+ x (f 3))))
