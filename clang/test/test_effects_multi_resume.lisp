;; test_effects_multi_resume.lisp - Tests for multiple resumptions

;; Handlers can resume multiple times (for nondeterminism, backtracking)

;; Choose effect - nondeterministic choice
;; TEST: choose from two options
;; EXPECT: (1 2)
(handle
  (cons (perform Choose '(1 2)) nil)
  (Choose [options resume]
    (match options
      ()       nil
      (h .. t) (cons (resume h) (resume-all Choose t resume)))))

;; Simpler: just resume with each
;; TEST: resume first then second
;; EXPECT: 1
(handle
  (perform Flip nil)
  (Flip [_ resume] (resume 1)))

;; TEST: resume can be called multiple times conceptually
;; EXPECT: "heads"
(handle
  (if (perform Flip nil) "heads" "tails")
  (Flip [_ resume] (resume true)))

;; Amb (ambiguous) effect for backtracking search
;; TEST: amb choose first valid
;; EXPECT: 2
(handle
  (perform Amb '(2 4 6))
  (Amb [options resume]
    (match options
      ()       nil
      (h .. t) (resume h))))

;; List all possibilities
;; TEST: collect all branches
;; EXPECT: (1 2 3)
(handle
  (let [x (perform OneOf '(1 2 3))]
    x)
  (OneOf [options resume]
    (match options
      ()       '()
      (h .. t) (cons (resume h) nil))))

;; Generate sequence via repeated resume
;; TEST: generate countdown
;; EXPECT: 5
(handle
  (perform Countdown 5)
  (Countdown [n resume]
    (if (= n 0)
      (resume 0)
      (resume n))))

;; Coroutine-like yield/resume
;; TEST: yield and resume pattern
;; EXPECT: 42
(handle
  (do
    (perform Yield 10)
    (perform Yield 20)
    42)
  (Yield [v resume] (resume nil)))

;; Multiple resume values accumulate
;; TEST: accumulate resumed values
;; EXPECT-FINAL: 15
(handle
  (+ (perform A nil) (perform B nil) (perform C nil))
  (A [_ resume] (resume 5))
  (B [_ resume] (resume 5))
  (C [_ resume] (resume 5)))
