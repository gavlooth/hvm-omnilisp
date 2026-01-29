;; test_let_seq.lisp - Tests for sequential let bindings

;; Sequential let where later bindings depend on earlier ones
;; TEST: sequential binding dependency
;; EXPECT: 3
(let ^:seq [x 1] [y (+ x 1)] [z (+ y 1)] z)

;; Sequential binding chain
;; TEST: chain of dependent computations
;; EXPECT: 16
(let ^:seq
  [a 2]
  [b (* a a)]      ; b = 4
  [c (* b b)]      ; c = 16
  c)

;; Sequential with expressions
(define pyramid-volume [base height]
  (let ^:seq
    [area (* base base)]
    [volume (* area height)]
    [third (/ volume 3)]
    third))

;; TEST: pyramid volume (base=3, height=6): 3*3=9, 9*6=54, 54/3=18
;; EXPECT: 18
(pyramid-volume 3 6)

;; Mixed independent and dependent
;; TEST: some parallel, some sequential
;; EXPECT-FINAL: 110
(let ^:seq
  [x 10]
  [y 100]
  [sum (+ x y)]    ; depends on both
  sum)
