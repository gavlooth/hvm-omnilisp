;; test_effects.lisp - Tests for algebraic effects

;; Basic effect handling
;; handle body with effect handler, resume continues with value

;; EXPECT-FINAL: 43
(handle
  (+ 1 (perform ask nothing))
  (ask [payload resume]
    (resume 42)))
