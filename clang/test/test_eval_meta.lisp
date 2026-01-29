;; test_eval_meta.lisp - Tests for meta-level evaluation

;; Eval-meta evaluates at meta-level
;; TEST: basic eval-meta
;; EXPECT: 6
(eval-meta '(+ 1 2 3))

;; TEST: eval-meta with environment
;; EXPECT: 10
(let [x 5]
  (eval-meta '(* x 2)))

;; TEST: eval-meta produces code
;; EXPECT: (+ 1 2)
(eval-meta '(list '+ 1 2))

;; TEST: eval-meta in macro
;; EXPECT: 6
(do
  (define-syntax compute-at-expand
    [(_ expr) (eval-meta expr)])
  (compute-at-expand (+ 1 2 3)))

;; TEST: eval-meta with quasiquote
;; EXPECT: (x 5)
(let [n 5]
  (eval-meta `(list 'x ,n)))

;; TEST: meta-eval (alias)
;; EXPECT: 15
(meta-eval '(fold + 0 '(1 2 3 4 5)))

;; TEST: eval at specific level
;; EXPECT: 3
(eval-at-level 1 '(+ 1 2))

;; TEST: compile-time eval
;; EXPECT: 120
(do
  (define-syntax factorial-ct
    [(_ n) (eval-meta `(fold * 1 (range 1 (+ ,n 1))))])
  (factorial-ct 5))

;; TEST: eval-meta preserves types
;; EXPECT-FINAL: true
(int? (eval-meta '(+ 1 2)))
