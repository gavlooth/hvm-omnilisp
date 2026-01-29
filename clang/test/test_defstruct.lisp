;; test_defstruct.lisp - Tests for struct definition

;; Defstruct creates a record-like structure
;; TEST: define struct
;; EXPECT: true
(defstruct Person
  name {String}
  age {Int})
(type? Person)

;; TEST: create struct instance
;; EXPECT: true
(let [p (Person :name "Alice" :age 30)]
  (instance-of? p Person))

;; TEST: struct field access
;; EXPECT: "Alice"
(let [p (Person :name "Alice" :age 30)]
  (:name p))

;; TEST: struct keyword access
;; EXPECT: 30
(let [p (Person :name "Alice" :age 30)]
  (:age p))

;; Struct with optional fields
(defstruct User
  id {Int}
  name {String}
  email {(Maybe String)})

;; TEST: struct with optional nil
;; EXPECT: nil
(let [u (User :id 1 :name "Bob")]
  (:email u))

;; TEST: struct with optional value
;; EXPECT: "bob@example.com"
(let [u (User :id 1 :name "Bob" :email "bob@example.com")]
  (:email u))

;; Update struct
;; TEST: struct update
;; EXPECT: 31
(let [p (Person :name "Alice" :age 30)
      p2 (assoc p :age 31)]
  (:age p2))

;; Struct equality
;; TEST: struct equality
;; EXPECT: true
(let [p1 (Person :name "Alice" :age 30)
      p2 (Person :name "Alice" :age 30)]
  (= p1 p2))

;; TEST: struct inequality
;; EXPECT-FINAL: false
(let [p1 (Person :name "Alice" :age 30)
      p2 (Person :name "Bob" :age 30)]
  (= p1 p2))
