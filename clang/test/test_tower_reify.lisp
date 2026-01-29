;; test_tower_reify.lisp - Tests for reify (to lower level)

;; Reify moves from code representation to execution
;; TEST: reify quoted code
;; EXPECT: 3
(reify '(+ 1 2))

;; TEST: reify literal
;; EXPECT: 42
(reify '42)

;; TEST: reify arithmetic
;; EXPECT: 20
(reify '(* 4 5))

;; Reify is inverse of reflect
;; TEST: reify-reflect roundtrip
;; EXPECT: 42
(reify (reflect 42))

;; TEST: reflect-reify roundtrip
;; EXPECT: '(+ 1 2)
(reflect (reify '(+ 1 2)))

;; Reify complex expressions
;; TEST: reify let
;; EXPECT: 10
(reify '(let [x 5] (* x 2)))

;; TEST: reify conditional
;; EXPECT: "yes"
(reify '(if true "yes" "no"))

;; TEST: reify function application
;; EXPECT: 6
(reify '((lambda [x] (* x 2)) 3))

;; Reify with environment
;; TEST: reify captures scope
;; EXPECT: 15
(let [a 5]
  (reify `(* ,a 3)))

;; Staged reification
;; TEST: reify staged code
;; EXPECT: 7
(let [code `(+ 3 4)]
  (reify code))

;; TEST: reify nested
;; EXPECT: 42
(reify (reify ''42))

;; TEST: reify match expression
;; EXPECT-FINAL: "two"
(reify '(match 2
          1 "one"
          2 "two"
          _ "other"))
