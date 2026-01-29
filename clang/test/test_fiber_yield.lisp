;; test_fiber_yield.lisp - Tests for yield

;; Yield suspends fiber and returns value
;; TEST: simple yield
;; EXPECT: 1
(let [f (fiber-new (lambda []
                     (yield 1)
                     (yield 2)
                     3))]
  (fiber-resume f))

;; TEST: yield second value
;; EXPECT: 2
(let [f (fiber-new (lambda []
                     (yield 1)
                     (yield 2)
                     3))]
  (fiber-resume f)
  (fiber-resume f))

;; TEST: yield final return
;; EXPECT: 3
(let [f (fiber-new (lambda []
                     (yield 1)
                     (yield 2)
                     3))]
  (fiber-resume f)
  (fiber-resume f)
  (fiber-resume f))

;; TEST: yield receives value
;; EXPECT: 20
(let [f (fiber-new (lambda []
                     (* (yield) 2)))]
  (fiber-resume f)
  (fiber-resume f 10))

;; Generator pattern
;; TEST: generator pattern
;; EXPECT: (1 2 3 4 5)
(let [gen (fiber-new (lambda []
                       (yield 1)
                       (yield 2)
                       (yield 3)
                       (yield 4)
                       (yield 5)))]
  (let collect [acc '()]
    (let [val (fiber-resume gen)]
      (if (= (fiber-status gen) :dead)
        (reverse acc)
        (collect (cons val acc))))))

;; TEST: yield nil
;; EXPECT: nil
(let [f (fiber-new (lambda [] (yield nil)))]
  (fiber-resume f))

;; TEST: yield in loop
;; EXPECT: 10
(let [f (fiber-new (lambda []
                     (let loop [i 0]
                       (if (< i 10)
                         (do (yield i)
                             (loop (+ i 1)))
                         i))))]
  (let step [n 10]
    (fiber-resume f)
    (if (> n 1)
      (step (- n 1))
      (fiber-resume f))))

;; TEST: yield preserves state
;; EXPECT-FINAL: 55
(let [f (fiber-new (lambda []
                     (let loop [i 0] [sum 0]
                       (if (> i 10)
                         sum
                         (do (yield sum)
                             (loop (+ i 1) (+ sum i)))))))]
  (let run [n 11]
    (if (= n 0)
      (fiber-resume f)
      (do (fiber-resume f)
          (run (- n 1))))))
