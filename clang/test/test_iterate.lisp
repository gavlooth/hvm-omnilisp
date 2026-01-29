;; test_iterate.lisp - Tests for iterate function

;; Iterate applies f repeatedly: x, f(x), f(f(x)), ...
;; Returns first n values
(define iterate-n [f] [x] [n]
  (let build [curr x] [count n] [acc '()]
    (if (<= count 0)
      (reverse acc)
      (build (f curr) (- count 1) (cons curr acc)))))

;; TEST: iterate with double
;; EXPECT: (1 2 4 8 16)
(iterate-n (lambda [x] (* x 2)) 1 5)

;; TEST: iterate with increment
;; EXPECT: (0 1 2 3 4)
(iterate-n (lambda [x] (+ x 1)) 0 5)

;; TEST: iterate with square (1, 1, 1, ...)
;; EXPECT: (1 1 1 1 1)
(iterate-n (lambda [x] (* x x)) 1 5)

;; TEST: iterate with decrement
;; EXPECT: (10 9 8 7 6)
(iterate-n (lambda [x] (- x 1)) 10 5)

;; Iterate zero times
;; TEST: iterate 0 times
;; EXPECT: ()
(iterate-n (lambda [x] (* x 2)) 1 0)

;; Iterate once
;; TEST: iterate 1 time
;; EXPECT: (5)
(iterate-n (lambda [x] (* x 2)) 5 1)

;; Powers of 2
;; TEST: powers of 2
;; EXPECT-FINAL: (1 2 4 8 16 32 64 128 256 512)
(iterate-n (lambda [x] (* x 2)) 1 10)
