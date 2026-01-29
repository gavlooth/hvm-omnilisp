;; test_effects_writer.lisp - Tests for Writer effect (tell)

;; Writer effect accumulates output (logs, etc.)
;; Operation: Tell (append to accumulated output)

;; TEST: single tell
;; EXPECT: "logged"
(handle
  (do
    (perform Tell "message")
    "logged")
  (Tell [msg resume] (resume nil)))

;; TEST: multiple tells
;; EXPECT: "done"
(handle
  (do
    (perform Tell "step 1")
    (perform Tell "step 2")
    (perform Tell "step 3")
    "done")
  (Tell [msg resume] (resume nil)))

;; Logging computation steps
(define log-and-compute [x]
  (handle
    (do
      (perform Tell "starting")
      (let [doubled (* x 2)]
        (perform Tell "doubled")
        (let [result (+ doubled 10)]
          (perform Tell "added 10")
          result)))
    (Tell [msg resume] (resume nil))))

;; TEST: log and compute
;; EXPECT: 30
(log-and-compute 10)

;; TEST: log with value
;; EXPECT: 42
(handle
  (let [x (perform Get nil)]
    (perform Tell x)
    x)
  (Get [_ resume] (resume 42))
  (Tell [msg resume] (resume nil)))

;; Trace effect for debugging
;; TEST: trace calls
;; EXPECT: "result"
(handle
  (do
    (perform Trace "enter function")
    (perform Trace "processing")
    (perform Trace "exit function")
    "result")
  (Trace [msg resume] (resume nil)))

;; TEST: tell with conditional
;; EXPECT-FINAL: true
(handle
  (do
    (if true
      (perform Tell "condition was true")
      (perform Tell "condition was false"))
    true)
  (Tell [msg resume] (resume nil)))
