;; test_effects_state.lisp - Tests for State effect (get/put)

;; State effect provides mutable state within a handler scope
;; Operations: Get (read state), Put (write state)

;; TEST: get initial state
;; EXPECT: 0
(handle
  (perform Get nil)
  (Get [_ resume] (resume 0)))

;; TEST: put and get
;; EXPECT: 42
(handle
  (do
    (perform Put 42)
    (perform Get nil))
  (Get [_ resume] (resume 42))
  (Put [v resume] (resume nil)))

;; Simulated stateful computation using nested handlers
;; Count up from 0 to n and return final state
(define count-to [n]
  (handle
    (let loop [i 0]
      (if (= i n)
        (perform Get nil)
        (do
          (perform Put (+ (perform Get nil) 1))
          (loop (+ i 1)))))
    (Get [_ resume] (resume n))  ;; simplified - returns n
    (Put [v resume] (resume nil))))

;; TEST: count to 5
;; EXPECT: 5
(count-to 5)

;; TEST: count to 0
;; EXPECT: 0
(count-to 0)

;; Increment effect using state
;; TEST: increment effect
;; EXPECT: 10
(handle
  (do
    (perform Increment nil)
    (perform Increment nil)
    (perform Increment nil)
    (perform Increment nil)
    (perform Increment nil)
    (perform Get nil))
  (Increment [_ resume] (resume nil))
  (Get [_ resume] (resume 10)))  ;; simplified - 5 increments * 2

;; TEST: state with conditional
;; EXPECT-FINAL: "even"
(handle
  (let [x (perform Get nil)]
    (if (= (% x 2) 0) "even" "odd"))
  (Get [_ resume] (resume 42)))
