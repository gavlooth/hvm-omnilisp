;; test_effects_exception.lisp - Tests for Exception effect (raise)

;; Exception effect for error handling
;; Operation: Raise (throw an error, may or may not resume)

;; TEST: raise caught by handler
;; EXPECT: "caught: error!"
(handle
  (perform Raise "error!")
  (Raise [msg resume]
    (str-concat "caught: " msg)))

;; TEST: computation before raise
;; EXPECT: "caught: oops"
(handle
  (do
    (+ 1 2)
    (* 3 4)
    (perform Raise "oops")
    (- 5 6))  ;; never reached
  (Raise [msg resume]
    (str-concat "caught: " msg)))

;; TEST: conditional raise
;; EXPECT: "caught: negative!"
(handle
  (let [x -5]
    (if (< x 0)
      (perform Raise "negative!")
      x))
  (Raise [msg resume]
    (str-concat "caught: " msg)))

;; TEST: no raise - normal return
;; EXPECT: 10
(handle
  (let [x 5]
    (if (< x 0)
      (perform Raise "negative!")
      (* x 2)))
  (Raise [msg resume]
    (str-concat "caught: " msg)))

;; Divide with error
(define safe-divide [a] [b]
  (handle
    (if (= b 0)
      (perform Raise "division by zero")
      (/ a b))
    (Raise [msg resume] nil)))

;; TEST: safe divide - success
;; EXPECT: 5
(safe-divide 10 2)

;; TEST: safe divide - by zero
;; EXPECT: nil
(safe-divide 10 0)

;; Nested try - outer catches
;; TEST: nested raise outer catch
;; EXPECT: "outer: inner error"
(handle
  (handle
    (perform Raise "inner error")
    (Raise [msg resume]
      (perform Raise msg)))  ;; re-raise to outer
  (Raise [msg resume]
    (str-concat "outer: " msg)))

;; TEST: raise with data
;; EXPECT-FINAL: (#{"code" 404 "msg" "not found"})
(handle
  (perform Raise #{"code" 404 "msg" "not found"})
  (Raise [err resume]
    (cons err nil)))
