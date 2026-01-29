;; test_fiber_spawn.lisp - Tests for fiber spawning

;; Fiber-spawn creates new concurrent fiber
;; TEST: spawn creates fiber
;; EXPECT: true
(fiber? (fiber-spawn (lambda [] 42)))

;; TEST: spawn runs body
;; EXPECT: 42
(let [f (fiber-spawn (lambda [] 42))]
  (fiber-join f))

;; TEST: spawn multiple fibers
;; EXPECT: (1 2 3)
(let [f1 (fiber-spawn (lambda [] 1))]
(let [f2 (fiber-spawn (lambda [] 2))]
(let [f3 (fiber-spawn (lambda [] 3))]
  (list (fiber-join f1) (fiber-join f2) (fiber-join f3)))))

;; TEST: spawn with closure
;; EXPECT: 10
(let [x 5]
  (let [f (fiber-spawn (lambda [] (* x 2)))]
    (fiber-join f)))

;; TEST: spawn-n creates multiple
;; EXPECT: 5
(length (fiber-spawn-n 5 (lambda [i] (* i i))))

;; TEST: spawn with args
;; EXPECT: 15
(let [f (fiber-spawn-with (lambda [a b] (+ a b)) 10 5)]
  (fiber-join f))

;; Spawn and communicate via channel
;; TEST: spawn producer-consumer
;; EXPECT: 10
(let [ch (channel)]
  (let [producer (fiber-spawn (lambda [] (channel-send ch 10)))]
  (let [consumer (fiber-spawn (lambda [] (channel-recv ch)))]
    (fiber-join consumer))))

;; TEST: spawn detached
;; EXPECT: :detached
(let [f (fiber-spawn-detached (lambda [] (sleep 100)))]
  (fiber-state f))

;; TEST: spawn pool
;; EXPECT-FINAL: (0 1 4 9 16)
(let [pool (fiber-pool 4)]
  (let [results (pool-map pool (lambda [x] (* x x)) '(0 1 2 3 4))]
    (pool-shutdown pool)
    results))
