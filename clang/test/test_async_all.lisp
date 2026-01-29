;; test_async_all.lisp - Tests for async combinators

;; Async-all waits for all async operations
;; TEST: async-all collects results
;; EXPECT: (1 2 3)
(async-all
  (list
    (async (lambda [] 1))
    (async (lambda [] 2))
    (async (lambda [] 3))))

;; TEST: async-all preserves order
;; EXPECT: (a b c d)
(async-all
  (list
    (async (lambda [] 'a))
    (async (lambda [] 'b))
    (async (lambda [] 'c))
    (async (lambda [] 'd))))

;; TEST: async-all empty list
;; EXPECT: ()
(async-all '())

;; TEST: async-all single item
;; EXPECT: (42)
(async-all (list (async (lambda [] 42))))

;; Async-any returns first completed
;; TEST: async-any returns first
;; EXPECT: :first
(async-any
  (list
    (async (lambda [] :first))
    (async (lambda [] (sleep 100) :slow))))

;; TEST: async-race (alias for any)
;; EXPECT: true
(let [result (async-race
               (list
                 (async (lambda [] :a))
                 (async (lambda [] :b))))]
  (or (= result :a) (= result :b)))

;; Async-map maps async over collection
;; TEST: async-map
;; EXPECT: (2 4 6 8)
(async-map (lambda [x] (* x 2)) '(1 2 3 4))

;; TEST: async-filter
;; EXPECT: (2 4)
(async-filter (lambda [x] (= (% x 2) 0)) '(1 2 3 4 5))

;; TEST: async-reduce
;; EXPECT: 15
(async-reduce + 0 '(1 2 3 4 5))

;; Async with concurrency limit
;; TEST: async-all with limit
;; EXPECT-FINAL: (1 2 3 4 5)
(async-all-limit 2
  (map (lambda [x] (async (lambda [] x))) '(1 2 3 4 5)))
