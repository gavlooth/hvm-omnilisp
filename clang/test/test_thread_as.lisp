;; test_thread_as.lisp - Tests for as-> threading macro

;; As-> threads with explicit binding
;; TEST: basic as->
;; EXPECT: 6
(as-> 1 x
  (+ x 1)
  (* x 3))

;; TEST: as-> position varies
;; EXPECT: 5
(as-> 10 n
  (/ n 2)
  (- 10 n))

;; TEST: as-> with function calls
;; EXPECT: "HELLO"
(as-> "hello" s
  (str-upper s))

;; TEST: as-> in middle position
;; EXPECT: "Value: 42"
(as-> 42 v
  (str v)
  (str "Value: " v))

;; TEST: as-> chained operations
;; EXPECT: 25
(as-> 5 x
  (* x x)
  (+ x 0))

;; TEST: as-> with collections
;; EXPECT: 3
(as-> '(1 2 3 4 5) lst
  (filter odd? lst)
  (length lst))

;; TEST: as-> nested
;; EXPECT: 100
(as-> 2 a
  (* a 5)
  (as-> a b
    (* b b)))

;; TEST: as-> preserves type
;; EXPECT: ("A" "B" "C")
(as-> '("a" "b" "c") items
  (map str-upper items))

;; TEST: as-> with dict
;; EXPECT-FINAL: "enhanced"
(as-> #{"key" "value"} d
  (assoc d "key" "enhanced")
  (get d "key"))
