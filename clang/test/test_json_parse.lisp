;; test_json_parse.lisp - Tests for JSON parsing

;; Json-parse converts JSON string to value
;; TEST: parse null
;; EXPECT: nil
(json-parse "null")

;; TEST: parse true
;; EXPECT: true
(json-parse "true")

;; TEST: parse false
;; EXPECT: false
(json-parse "false")

;; TEST: parse integer
;; EXPECT: 42
(json-parse "42")

;; TEST: parse negative
;; EXPECT: -123
(json-parse "-123")

;; TEST: parse float
;; EXPECT: 3.14
(json-parse "3.14")

;; TEST: parse string
;; EXPECT: "hello"
(json-parse "\"hello\"")

;; TEST: parse string with escape
;; EXPECT: "hello\nworld"
(json-parse "\"hello\\nworld\"")

;; TEST: parse empty array
;; EXPECT: []
(json-parse "[]")

;; TEST: parse array
;; EXPECT: [1 2 3]
(json-parse "[1, 2, 3]")

;; TEST: parse nested array
;; EXPECT: [[1 2] [3 4]]
(json-parse "[[1, 2], [3, 4]]")

;; TEST: parse empty object
;; EXPECT: #{}
(json-parse "{}")

;; TEST: parse object
;; EXPECT: #{"name" "Alice" "age" 30}
(json-parse "{\"name\": \"Alice\", \"age\": 30}")

;; TEST: parse nested object
;; EXPECT: #{"user" #{"name" "Bob"}}
(json-parse "{\"user\": {\"name\": \"Bob\"}}")

;; TEST: parse mixed
;; EXPECT-FINAL: #{"items" [1 2 3] "active" true}
(json-parse "{\"items\": [1, 2, 3], \"active\": true}")
