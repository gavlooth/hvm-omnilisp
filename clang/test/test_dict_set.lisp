;; test_dict_set.lisp - Tests for dict-set and dict-set-in

;; Basic dict-set (functional update)
;; TEST: set new key
;; EXPECT: #{:a 1 :b 2}
(dict-set #{:a 1} :b 2)

;; TEST: update existing key
;; EXPECT: #{:a 10 :b 2}
(dict-set #{:a 1 :b 2} :a 10)

;; TEST: set on empty dict
;; EXPECT: #{:x 42}
(dict-set #{} :x 42)

;; Multiple sets
;; TEST: chain dict-set
;; EXPECT: #{:a 1 :b 2 :c 3}
(dict-set (dict-set (dict-set #{} :a 1) :b 2) :c 3)

;; Nested set with dict-set-in
(define person #{:name "Alice" :address #{:city "NYC" :zip "10001"}})

;; TEST: set-in update nested value
;; EXPECT: #{:name "Alice" :address #{:city "Boston" :zip "10001"}}
(dict-set-in person [:address :city] "Boston")

;; TEST: set-in update top level
;; EXPECT: #{:name "Bob" :address #{:city "NYC" :zip "10001"}}
(dict-set-in person [:name] "Bob")

;; TEST: set-in create new nested key
;; EXPECT-FINAL: #{:name "Alice" :address #{:city "NYC" :zip "10001" :country "USA"}}
(dict-set-in person [:address :country] "USA")
