;; test_fiber_resume.lisp - Tests for fiber resume

;; Fiber-resume continues fiber execution
;; TEST: resume fiber
;; EXPECT: 42
(let [f (fiber-new (lambda [] 42))]
  (fiber-resume f))

;; TEST: resume fiber with computation
;; EXPECT: 15
(let [f (fiber-new (lambda [] (+ 1 2 3 4 5)))]
  (fiber-resume f))

;; TEST: fiber status after resume
;; EXPECT: :dead
(let [f (fiber-new (lambda [] 42))]
  (fiber-resume f)
  (fiber-status f))

;; Resume with value
;; TEST: resume with value
;; EXPECT: 100
(let [f (fiber-new (lambda []
                     (let [x (yield)]
                       (* x 2))))]
  (fiber-resume f)       ;; run to yield
  (fiber-resume f 50))   ;; resume with value

;; TEST: multiple resume/yield
;; EXPECT: 6
(let [f (fiber-new (lambda []
                     (let [a (yield 1)]
                       (let [b (yield 2)]
                         (+ a b)))))]
  (fiber-resume f)       ;; yields 1
  (fiber-resume f 2)     ;; yields 2, a=2
  (fiber-resume f 4))    ;; returns a+b=2+4=6

;; TEST: resume dead fiber
;; EXPECT: nil
(let [f (fiber-new (lambda [] 42))]
  (fiber-resume f)  ;; completes
  (fiber-resume f)) ;; dead, returns nil

;; TEST: fiber return value
;; EXPECT: "done"
(let [f (fiber-new (lambda [] "done"))]
  (fiber-resume f))

;; Resume with error handling
;; TEST: fiber with error
;; EXPECT-FINAL: "caught"
(let [f (fiber-new (lambda []
                     (perform Raise "error")))]
  (handle
    (fiber-resume f)
    (Raise [_ resume] "caught")))
