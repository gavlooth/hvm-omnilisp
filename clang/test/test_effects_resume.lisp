;; test_effects_resume.lisp - Tests for resumption with values

;; Handler can resume with any value, changing computation flow

;; TEST: resume with different value
;; EXPECT: 100
(handle
  (+ 50 (perform Get nil))
  (Get [_ resume] (resume 50)))

;; TEST: resume with transformed value
;; EXPECT: 20
(handle
  (perform Double 10)
  (Double [x resume] (resume (* x 2))))

;; TEST: handler transforms multiple
;; EXPECT: (2 4 6)
(handle
  (cons (perform Transform 1)
        (cons (perform Transform 2)
              (cons (perform Transform 3) nil)))
  (Transform [x resume] (resume (* x 2))))

;; TEST: resume changes control flow
;; EXPECT: "yes"
(handle
  (if (perform Check nil) "yes" "no")
  (Check [_ resume] (resume true)))

;; Middleware pattern - wrap value
;; TEST: wrap in container
;; EXPECT: #{"value" 42 "status" "ok"}
(handle
  (perform Wrap 42)
  (Wrap [v resume]
    (resume #{"value" v "status" "ok"})))

;; Chain resumptions
;; TEST: chained resumptions
;; EXPECT: 16
(handle
  (let [a (perform Get nil)]
    (let [b (perform Get nil)]
      (+ a b)))
  (Get [_ resume] (resume 8)))

;; Resume with computed value
;; TEST: resume with computation
;; EXPECT: 120
(handle
  (perform Factorial 5)
  (Factorial [n resume]
    (let fact [x n] [acc 1]
      (if (<= x 1)
        (resume acc)
        (fact (- x 1) (* acc x))))))

;; TEST: resume nil
;; EXPECT: nil
(handle
  (perform Optional nil)
  (Optional [_ resume] (resume nil)))

;; TEST: resume preserves type
;; EXPECT-FINAL: ("hello" "world")
(handle
  (cons (perform GetString nil)
        (cons (perform GetString nil) nil))
  (GetString [_ resume]
    (resume (if (= 1 1) "hello" "world"))))
