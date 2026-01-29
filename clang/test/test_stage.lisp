;; test_stage.lisp - Tests for staged computation

;; Stage delays computation to next level
;; TEST: stage simple
;; EXPECT: '(+ 1 2)
(stage (+ 1 2))

;; TEST: stage arithmetic
;; EXPECT: '(* 3 4)
(stage (* 3 4))

;; TEST: stage with splice
;; EXPECT: (+ 5 10)
(let [a 5] [b 10]
  (stage (+ ~a ~b)))

;; Stage for code generation
;; TEST: generate addition code
;; EXPECT: '(+ x y)
(define gen-add [x] [y]
  (stage (+ ~x ~y)))
(gen-add 'x 'y)

;; TEST: stage preserves structure
;; EXPECT: '(let [x 5] (* x 2))
(stage (let [x 5] (* x 2)))

;; Multi-stage
;; TEST: two-stage
;; EXPECT: ''(+ 1 2)
(stage (stage (+ 1 2)))

;; Stage with computed values
;; TEST: stage computed
;; EXPECT: '(* 10 2)
(let [n (* 5 2)]
  (stage (* ~n 2)))

;; Run staged code
;; TEST: run staged
;; EXPECT: 3
(run (stage (+ 1 2)))

;; TEST: stage function
;; EXPECT: '(lambda [x] (* x 2))
(stage (lambda [x] (* x 2)))

;; TEST: stage in loop generates unrolled code
;; EXPECT: '(+ (+ (+ 0 1) 2) 3)
(let gen-sum [n 3] [acc '0]
  (if (= n 0)
    acc
    (gen-sum (- n 1) `(+ ,acc ,n))))

;; TEST: stage conditional
;; EXPECT-FINAL: '(if true 1 2)
(stage (if true 1 2))
