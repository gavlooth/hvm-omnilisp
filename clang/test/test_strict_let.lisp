;; test_strict_let.lisp - Tests for ^:strict metadata on let bindings

;; TEST: basic let (lazy by default)
;; EXPECT: 5
(let [x 2] [y 3] (+ x y))

;; TEST: strict let binding
;; EXPECT: 10
(let [^:strict x 5] [y 5] (+ x y))

;; TEST: mixed strict and lazy bindings
;; EXPECT: 30
(let [^:strict a 10]
     [b 20]
  (+ a b))

;; TEST: multiple strict bindings
;; EXPECT: 6
(let [^:strict x 1]
     [^:strict y 2]
     [^:strict z 3]
  (+ x (+ y z)))

;; EXPECT-FINAL: 15
(let [^:strict sum 15] sum)
