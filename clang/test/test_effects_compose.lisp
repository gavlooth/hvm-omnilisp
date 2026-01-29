;; test_effects_compose.lisp - Tests for effect handler composition

;; Composing multiple effect handlers

;; State + Reader composition
;; TEST: combine state and reader
;; EXPECT: "config: production"
(handle
  (handle
    (str-concat "config: " (perform Ask nil))
    (Ask [_ resume] (resume (perform Get nil))))
  (Get [_ resume] (resume "production")))

;; Reader + Writer composition
;; TEST: read config and log
;; EXPECT: "done"
(handle
  (handle
    (do
      (perform Tell (perform Ask nil))
      "done")
    (Ask [_ resume] (resume "value")))
  (Tell [msg resume] (resume nil)))

;; State + Exception composition
;; TEST: state with error handling
;; EXPECT: "error at state 5"
(handle
  (handle
    (do
      (perform Put 5)
      (perform Raise "error"))
    (Raise [msg resume]
      (str-concat "error at state " (int->str (perform Get nil)))))
  (Get [_ resume] (resume 5))
  (Put [v resume] (resume nil)))

;; All three: Reader + Writer + Exception
;; TEST: three effects composed
;; EXPECT: "handled"
(handle
  (handle
    (handle
      (do
        (let [config (perform Ask nil)]
          (perform Tell config)
          (if (= config "error")
            (perform Raise "bad config")
            config)))
        "handled")
      (Ask [_ resume] (resume "ok")))
    (Tell [msg resume] (resume nil)))
  (Raise [msg resume] (resume "error")))

;; Effect delegation pattern
;; TEST: delegate to outer handler
;; EXPECT: "delegated: inner"
(handle
  (handle
    (perform Forward "inner")
    (Forward [msg resume]
      (perform Outer msg)))  ;; delegate to outer
  (Outer [msg resume]
    (resume (str-concat "delegated: " msg))))

;; Transformer pattern
;; TEST: transform then forward
;; EXPECT-FINAL: "HELLO"
(handle
  (handle
    (perform Process "hello")
    (Process [msg resume]
      (perform Output (str-upper msg))))
  (Output [msg resume] (resume msg)))
