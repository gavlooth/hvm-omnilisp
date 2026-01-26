; OmniLisp Basic Test File
; Tests core language features

; Basic arithmetic
(+ 1 2)                          ; → 3
(* (+ 1 2) (- 5 3))              ; → 6
(/ 10 3)                         ; → 3

; Boolean operations
(and true false)                 ; → false
(or true false)                  ; → true
(not false)                      ; → true

; Comparison
(= 1 1)                          ; → true
(< 1 2)                          ; → true
(> 3 2)                          ; → true

; Let binding
(let [x 10]
  (+ x 5))                       ; → 15

; Nested let
(let [x 10]
  (let [y 20]
    (+ x y)))                    ; → 30

; Lambda
(lambda [x] (+ x 1))             ; → closure

; Lambda application
((lambda [x] (+ x 1)) 5)         ; → 6

; Named function (define)
(define square [x {Int}]
  (* x x))

(square 5)                       ; → 25

; Recursive function
(define factorial [n {Int}]
  (if (= n 0)
    1
    (* n (factorial (- n 1)))))

(factorial 5)                    ; → 120

; List operations
(cons 1 (cons 2 (cons 3 nil)))   ; → (1 2 3)
(first (cons 1 (cons 2 nil)))    ; → 1
(rest (cons 1 (cons 2 nil)))     ; → (2)

; Pattern matching
(define length [xs {List}]
  (match xs
    [[] 0]
    [[h .. t] (+ 1 (length t))]))

(length (cons 1 (cons 2 (cons 3 nil))))  ; → 3

; Pattern matching with guard
(define abs [n {Int}]
  (match n
    [x :when (< x 0) (- 0 x)]
    [x x]))

(abs -5)                         ; → 5
(abs 5)                          ; → 5

; Type annotations (for documentation/dispatch)
(define add {Int} [x {Int}] [y {Int}]
  (+ x y))

; Algebraic effects (example structure)
; (handle (+ 1 (perform ask nothing))
;   (ask [_ resume] (resume 42)))  ; → 43

; Print result
(+ 100 23)                       ; → 123

