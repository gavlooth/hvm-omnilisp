;; test_effects_finally.lisp - Tests for effect cleanup/finally

;; Finally ensures cleanup runs
;; TEST: finally runs on success
;; EXPECT: (:done :cleaned)
(let [log '()]
  (handle
    (do
      (set! log (cons :done log))
      :result)
    :finally (set! log (cons :cleaned log)))
  (reverse log))

;; TEST: finally runs on effect
;; EXPECT: :cleaned
(let [state nil]
  (handle
    (do
      (perform Fail "error")
      :never-reached)
    (Fail [e k] :caught)
    :finally (set! state :cleaned))
  state)

;; TEST: with-resource pattern
;; EXPECT: :closed
(let [resource-state nil]
  (with-resource [r (do (set! resource-state :open) r)]
    :finally (set! resource-state :closed)
    (do :using-resource))
  resource-state)

;; TEST: nested finally
;; EXPECT: (:outer :inner)
(let [log '()]
  (handle
    (handle
      :inner-result
      :finally (set! log (cons :inner log)))
    :finally (set! log (cons :outer log)))
  (reverse log))

;; TEST: finally with resumption
;; EXPECT: 42
(handle
  (+ (perform GetValue nil) 2)
  (GetValue [_ k] (k 40))
  :finally nil)

;; TEST: ensure (alias for finally)
;; EXPECT: :ensured
(let [flag nil]
  (ensure
    (do (set! flag :ensured) :body)
    :finally nil)
  flag)

;; TEST: unwind-protect style
;; EXPECT-FINAL: (:started :completed)
(let [log '()]
  (unwind-protect
    (do
      (set! log (cons :started log))
      :work)
    (set! log (cons :completed log)))
  (reverse log))
