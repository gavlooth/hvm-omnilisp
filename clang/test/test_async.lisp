;; test_async.lisp - Tests for async/await

;; Async creates a promise/future
;; TEST: async returns promise
;; EXPECT: true
(promise? (async 42))

;; TEST: await gets value
;; EXPECT: 42
(await (async 42))

;; TEST: async with computation
;; EXPECT: 15
(await (async (+ 1 2 3 4 5)))

;; TEST: async preserves exceptions
;; EXPECT: "caught"
(handle
  (await (async (perform Raise "error")))
  (Raise [_ resume] "caught"))

;; Multiple async
;; TEST: multiple async
;; EXPECT: (1 2 3)
(list (await (async 1))
      (await (async 2))
      (await (async 3)))

;; Async-all waits for all
;; TEST: async-all
;; EXPECT: (1 2 3)
(await (async-all (async 1) (async 2) (async 3)))

;; TEST: async-all empty
;; EXPECT: ()
(await (async-all))

;; Async-race returns first
;; TEST: async-race
;; EXPECT: true
(let [result (await (async-race (async 1) (async 2) (async 3)))]
  (member? result '(1 2 3)))

;; Async with delay (simulated)
;; TEST: async with sleep
;; EXPECT: "done"
(await (async
         (sleep 10)
         "done"))

;; Then chaining
;; TEST: promise then
;; EXPECT: 84
(await (promise-then (async 42) (lambda [x] (* x 2))))

;; TEST: promise chain
;; EXPECT: 8
(await (promise-then
         (promise-then (async 2) (lambda [x] (* x 2)))
         (lambda [x] (* x 2))))

;; Catch errors
;; TEST: promise catch
;; EXPECT-FINAL: "recovered"
(await (promise-catch
         (async (perform Raise "error"))
         (lambda [err] "recovered")))
