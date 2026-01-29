;; test_deftype.lisp - Tests for custom type definition

;; Define a simple type
;; TEST: define type
;; EXPECT: true
(deftype Point [x {Int}] [y {Int}])
(type? Point)

;; TEST: create instance
;; EXPECT: true
(let [p (Point 3 4)]
  (instance-of? p Point))

;; TEST: access fields
;; EXPECT: 3
(let [p (Point 3 4)]
  (get p :x))

;; TEST: access y field
;; EXPECT: 4
(let [p (Point 3 4)]
  (get p :y))

;; Type with methods
(deftype Circle [center {Point}] [radius {Float}])

;; TEST: nested type
;; EXPECT: true
(let [c (Circle (Point 0 0) 5.0)]
  (instance-of? c Circle))

;; TEST: nested field access
;; EXPECT: 0
(let [c (Circle (Point 0 0) 5.0)]
  (get (get c :center) :x))

;; Type with default values
(deftype Config
  [host {String} "localhost"]
  [port {Int} 8080])

;; TEST: type with defaults
;; EXPECT: "localhost"
(let [cfg (Config)]
  (get cfg :host))

;; TEST: override default
;; EXPECT: "example.com"
(let [cfg (Config "example.com" 443)]
  (get cfg :host))

;; Pattern match on type
;; TEST: pattern match type
;; EXPECT-FINAL: 7
(let [p (Point 3 4)]
  (match p
    (Point x y) (+ x y)))
