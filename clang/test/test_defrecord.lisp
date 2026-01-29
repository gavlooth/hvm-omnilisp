;; test_defrecord.lisp - Tests for record type definition

;; Defrecord creates immutable data types
;; TEST: basic record
;; EXPECT: true
(do
  (defrecord Person [name {String}] [age {Int}])
  (record? (Person "Alice" 30)))

;; TEST: record field access
;; EXPECT: "Alice"
(do
  (defrecord Person [name {String}] [age {Int}])
  (let [p (Person "Alice" 30)]
    (get p :name)))

;; TEST: record with defaults
;; EXPECT: 0
(do
  (defrecord Counter [value {Int} :default 0])
  (get (Counter) :value))

;; TEST: record constructor
;; EXPECT: true
(do
  (defrecord Point [x {Int}] [y {Int}])
  (let [p (Point 3 4)]
    (and (= (get p :x) 3) (= (get p :y) 4))))

;; TEST: record update (returns new)
;; EXPECT: (1 99)
(do
  (defrecord Point [x {Int}] [y {Int}])
  (let [p1 (Point 1 2)]
  (let [p2 (assoc p1 :y 99)]
    (list (get p1 :y) (get p2 :y)))))

;; TEST: record pattern match
;; EXPECT: 7
(do
  (defrecord Point [x {Int}] [y {Int}])
  (match (Point 3 4)
    (Point x y) (+ x y)))

;; TEST: record equality
;; EXPECT: true
(do
  (defrecord Point [x {Int}] [y {Int}])
  (= (Point 1 2) (Point 1 2)))

;; TEST: record to-map
;; EXPECT: #{"x" 1 "y" 2}
(do
  (defrecord Point [x {Int}] [y {Int}])
  (record->map (Point 1 2)))

;; TEST: record from-map
;; EXPECT: true
(do
  (defrecord Point [x {Int}] [y {Int}])
  (let [p (map->Point #{"x" 1 "y" 2})]
    (= p (Point 1 2))))

;; TEST: record validation
;; EXPECT-FINAL: :error
(do
  (defrecord Positive [n {Int & (> _ 0)}])
  (handle
    (Positive -5)
    (TypeError [e k] :error)))
