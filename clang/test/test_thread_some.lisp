;; test_thread_some.lisp - Tests for some-> threading (nil-safe)

;; Some-> stops on nil
;; TEST: basic some->
;; EXPECT: 6
(some-> 1
  (+ 1)
  (* 3))

;; TEST: some-> stops on nil
;; EXPECT: nil
(some-> nil
  (+ 1)
  (* 3))

;; TEST: some-> nil in middle
;; EXPECT: nil
(some-> #{"a" 1}
  (get "b")  ;; returns nil
  (* 2))

;; TEST: some-> continues on value
;; EXPECT: 2
(some-> #{"a" 1}
  (get "a")
  (* 2))

;; TEST: some->> thread last nil-safe
;; EXPECT: nil
(some->> nil
  (map inc)
  (filter odd?))

;; TEST: some->> with value
;; EXPECT: (3 5)
(some->> '(1 2 3 4)
  (map inc)
  (filter odd?))

;; TEST: some-> nested access
;; EXPECT: 3
(some-> #{"user" #{"profile" #{"age" 3}}}
  (get "user")
  (get "profile")
  (get "age"))

;; TEST: some-> nested nil
;; EXPECT: nil
(some-> #{"user" nil}
  (get "user")
  (get "profile"))

;; TEST: some-> with predicate
;; EXPECT-FINAL: nil
(some-> 5
  ((lambda [x] (if (> x 10) x nil)))
  (* 2))
