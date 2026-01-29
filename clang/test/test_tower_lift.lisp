;; test_tower_lift.lisp - Tests for lift (value to code)

;; Lift converts a value to its code representation
;; TEST: lift integer
;; EXPECT: 42
(lift 42)

;; TEST: lift string
;; EXPECT: "hello"
(lift "hello")

;; TEST: lift boolean
;; EXPECT: true
(lift true)

;; TEST: lift list
;; EXPECT: '(1 2 3)
(lift '(1 2 3))

;; TEST: lift quoted symbol
;; EXPECT: 'foo
(lift 'foo)

;; Lift preserves structure
;; TEST: lift nested
;; EXPECT: '((1 2) (3 4))
(lift '((1 2) (3 4)))

;; TEST: lift dict
;; EXPECT: #{"a" 1}
(lift #{"a" 1})

;; Lift in staged computation
;; TEST: lift in quasiquote
;; EXPECT: (+ 5 10)
(let [a 5] [b 10]
  `(+ ,(lift a) ,(lift b)))

;; TEST: lift computed value
;; EXPECT: 15
(let [x (+ 10 5)]
  (lift x))

;; Lift preserves type info
;; TEST: lift maintains type
;; EXPECT: true
(int? (lift 42))

;; TEST: lift maintains string type
;; EXPECT: true
(string? (lift "test"))

;; TEST: lift nil
;; EXPECT-FINAL: nil
(lift nil)
