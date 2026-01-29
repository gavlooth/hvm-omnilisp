;; test_deftype_alias.lisp - Tests for type aliases

;; Deftype-alias creates type synonym
;; TEST: basic type alias
;; EXPECT: true
(do
  (deftype-alias UserId {Int})
  (type-alias? UserId))

;; TEST: alias for compound type
;; EXPECT: true
(do
  (deftype-alias IntList {(List Int)})
  (check-type '(1 2 3) {IntList}))

;; TEST: alias with parameters
;; EXPECT: true
(do
  (deftype-alias Pair [A B] {(Tuple A B)})
  (check-type '(1 "hello") {(Pair Int String)}))

;; TEST: nested alias
;; EXPECT: true
(do
  (deftype-alias Id {Int})
  (deftype-alias IdList {(List Id)})
  (check-type '(1 2 3) {IdList}))

;; TEST: alias in function signature
;; EXPECT: 10
(do
  (deftype-alias Count {Int})
  (define double-count [n {Count}] {Count}
    (* n 2))
  (double-count 5))

;; TEST: alias transparency
;; EXPECT: true
(do
  (deftype-alias MyInt {Int})
  (let [x {MyInt} 5]
    (int? x)))

;; TEST: alias with constraint
;; EXPECT: true
(do
  (deftype-alias PositiveInt {Int & (> _ 0)})
  (check-type 5 {PositiveInt}))

;; TEST: alias lookup
;; EXPECT: {Int}
(do
  (deftype-alias Num {Int})
  (expand-type-alias Num))

;; TEST: unalias (force original type)
;; EXPECT-FINAL: true
(do
  (deftype-alias Wrapped {Int})
  (= (unalias-type {Wrapped}) {Int}))
