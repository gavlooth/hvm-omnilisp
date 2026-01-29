;; test_fiber_new.lisp - Tests for fiber creation

;; Fiber-new creates a new fiber (lightweight thread)
;; TEST: create fiber
;; EXPECT: true
(fiber? (fiber-new (lambda [] 42)))

;; TEST: fiber is suspended initially
;; EXPECT: :suspended
(fiber-status (fiber-new (lambda [] 42)))

;; TEST: fiber holds function
;; EXPECT: true
(let [f (fiber-new (lambda [] "result"))]
  (fiber? f))

;; TEST: fiber with computation
;; EXPECT: true
(let [f (fiber-new (lambda []
                     (+ 1 2 3 4 5)))]
  (fiber? f))

;; TEST: multiple fibers
;; EXPECT: 3
(let [f1 (fiber-new (lambda [] 1))
      f2 (fiber-new (lambda [] 2))
      f3 (fiber-new (lambda [] 3))]
  (length (filter fiber? (list f1 f2 f3))))

;; Fiber with arguments
;; TEST: fiber-new with args
;; EXPECT: true
(let [f (fiber-new (lambda [x] (* x 2)) 21)]
  (fiber? f))

;; Fiber identity
;; TEST: fibers have unique ids
;; EXPECT: false
(let [f1 (fiber-new (lambda [] 1))
      f2 (fiber-new (lambda [] 2))]
  (= (fiber-id f1) (fiber-id f2)))

;; TEST: same fiber same id
;; EXPECT: true
(let [f (fiber-new (lambda [] 1))]
  (= (fiber-id f) (fiber-id f)))

;; TEST: fiber name
;; EXPECT-FINAL: "worker"
(let [f (fiber-new (lambda [] 1) :name "worker")]
  (fiber-name f))
