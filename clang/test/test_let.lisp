;; test_let.lisp - Tests for let bindings

;; Basic let with single binding
;; TEST: single binding
;; EXPECT: 10
(let [x 10] x)

;; Multiple bindings (parallel evaluation)
;; TEST: multiple independent bindings
;; EXPECT: 30
(let [a 10] [b 20] (+ a b))

;; Let with expressions
;; TEST: binding to expression result
;; EXPECT: 25
(let [x (* 5 5)] x)

;; Nested let
;; TEST: nested let bindings
;; EXPECT: 60
(let [x 10]
  (let [y 20]
    (let [z 30]
      (+ x (+ y z)))))

;; Let with function body
(define circle-area [r]
  (let [pi 3]
    (* pi (* r r))))

;; TEST: circle area with r=2
;; EXPECT: 12
(circle-area 2)

;; Let shadowing
;; TEST: inner binding shadows outer
;; EXPECT-FINAL: 100
(let [x 10]
  (let [x 100]
    x))
