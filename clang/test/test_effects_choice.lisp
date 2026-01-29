;; test_effects_choice.lisp - Effect for choice/branching

;; Choose effect returns first option
;; EXPECT-FINAL: 10
(handle
  (* 2 (perform choose (cons 5 (cons 10 nil))))
  (choose [options resume]
    (resume (match options (h .. t) h))))
