;; test_type_intersection.lisp - Tests for intersection types

;; Intersection types require all component types
;; TEST: basic intersection
;; EXPECT: true
(do
  (defprotocol Printable (to-string [self]))
  (defprotocol Comparable (compare [self other]))
  (deftype Num [value {Int}]
    Printable (to-string [self] (str (get self :value)))
    Comparable (compare [self other] (- (get self :value) (get other :value))))
  (check-type (Num 5) {Printable & Comparable}))

;; TEST: intersection narrows type
;; EXPECT: true
(let [x {Int & (> _ 0) & (< _ 10)} 5]
  (and (> x 0) (< x 10)))

;; TEST: intersection with alias
;; EXPECT: true
(do
  (deftype-alias Positive {Int & (> _ 0)})
  (deftype-alias Small {Int & (< _ 100)})
  (check-type 50 {Positive & Small}))

;; TEST: function accepting intersection
;; EXPECT: 10
(do
  (defprotocol Addable (add [self n]))
  (defprotocol Multipliable (mul [self n]))
  (deftype MyNum [v {Int}]
    Addable (add [self n] (MyNum (+ (get self :v) n)))
    Multipliable (mul [self n] (MyNum (* (get self :v) n))))
  (define process [x {Addable & Multipliable}]
    (get (mul (add x 2) 2) :v))
  (process (MyNum 3)))

;; TEST: intersection creates new type
;; EXPECT: true
(type? {Int & (> _ 0)})

;; TEST: intersection vs union
;; EXPECT: true
(do
  (deftype-alias NumOrStr {Int | String})
  (deftype-alias Both {Int & (> _ 0)})
  (and (check-type 5 {NumOrStr})
       (check-type 5 {Both})))

;; TEST: empty intersection (impossible)
;; EXPECT-FINAL: false
(check-type 5 {(> _ 10) & (< _ 3)})
