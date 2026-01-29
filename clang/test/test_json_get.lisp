;; test_json_get.lisp - Tests for JSON access

;; Json-get accesses JSON values
;; TEST: get object key
;; EXPECT: "Alice"
(let [j (json-parse "{\"name\": \"Alice\"}")]
  (json-get j "name"))

;; TEST: get nested
;; EXPECT: "NYC"
(let [j (json-parse "{\"address\": {\"city\": \"NYC\"}}")]
  (json-get-in j '("address" "city")))

;; TEST: get array index
;; EXPECT: 2
(let [j (json-parse "[1, 2, 3]")]
  (json-get j 1))

;; TEST: get missing key
;; EXPECT: nil
(let [j (json-parse "{\"a\": 1}")]
  (json-get j "b"))

;; TEST: get with default
;; EXPECT: "default"
(let [j (json-parse "{\"a\": 1}")]
  (json-get j "b" "default"))

;; Json-type returns the JSON type
;; TEST: type of null
;; EXPECT: "null"
(json-type (json-parse "null"))

;; TEST: type of boolean
;; EXPECT: "boolean"
(json-type (json-parse "true"))

;; TEST: type of number
;; EXPECT: "number"
(json-type (json-parse "42"))

;; TEST: type of string
;; EXPECT: "string"
(json-type (json-parse "\"hello\""))

;; TEST: type of array
;; EXPECT: "array"
(json-type (json-parse "[1,2,3]"))

;; TEST: type of object
;; EXPECT: "object"
(json-type (json-parse "{\"a\":1}"))

;; Json predicates
;; TEST: json-null?
;; EXPECT: true
(json-null? (json-parse "null"))

;; TEST: json-array?
;; EXPECT: true
(json-array? (json-parse "[1,2]"))

;; TEST: json-object?
;; EXPECT-FINAL: true
(json-object? (json-parse "{\"a\":1}"))
