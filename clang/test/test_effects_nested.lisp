;; test_effects_nested.lisp - Tests for nested effect handlers

;; Inner handler catches its effect, outer catches different effect

;; TEST: nested handlers - each catches own
;; EXPECT: 30
(handle
  (handle
    (+ (perform GetA nil) (perform GetB nil))
    (GetB [_ resume] (resume 20)))
  (GetA [_ resume] (resume 10)))

;; TEST: inner effect doesn't escape
;; EXPECT: 100
(handle
  (handle
    (* (perform Inner nil) 10)
    (Inner [_ resume] (resume 10)))
  (Outer [_ resume] (resume 999)))  ;; never triggered

;; TEST: outer catches when inner doesn't handle
;; EXPECT: "outer caught"
(handle
  (handle
    (perform Escape nil)
    (Other [_ resume] (resume "inner")))
  (Escape [_ resume] (resume "outer caught")))

;; Layered state
;; TEST: layered state handlers
;; EXPECT: 15
(handle
  (handle
    (+ (perform Layer1 nil) (perform Layer2 nil))
    (Layer2 [_ resume] (resume 10)))
  (Layer1 [_ resume] (resume 5)))

;; Nested with same effect name - inner shadows
;; TEST: inner shadows outer
;; EXPECT: "inner"
(handle
  (handle
    (perform Get nil)
    (Get [_ resume] (resume "inner")))
  (Get [_ resume] (resume "outer")))

;; Deeply nested
;; TEST: three levels deep
;; EXPECT: 111
(handle
  (handle
    (handle
      (+ (perform A nil) (+ (perform B nil) (perform C nil)))
      (C [_ resume] (resume 1)))
    (B [_ resume] (resume 10)))
  (A [_ resume] (resume 100)))

;; Mixed nesting
;; TEST: alternating handler types
;; EXPECT-FINAL: "result: 42"
(handle
  (handle
    (str-concat "result: " (int->str (perform GetNum nil)))
    (GetNum [_ resume] (resume 42)))
  (Log [msg resume] (resume nil)))
