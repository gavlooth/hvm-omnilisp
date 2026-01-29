;; test_timed.lisp - Tests for timing utilities

;; Timing utilities measure execution
;; TEST: time macro prints duration
;; EXPECT: 6
(time (+ 1 2 3))

;; TEST: time returns result
;; EXPECT: 10
(time (* 2 5))

;; TEST: timed returns duration and result
;; EXPECT: true
(let [result (timed (+ 1 2))]
  (and (map? result)
       (= (get result :value) 3)
       (number? (get result :duration-ms))))

;; TEST: timed on slow operation
;; EXPECT: true
(let [result (timed (do (range 1000) :done))]
  (>= (get result :duration-ms) 0))

;; TEST: benchmark runs multiple times
;; EXPECT: true
(let [result (benchmark 100 (+ 1 2))]
  (map? result))

;; TEST: benchmark includes stats
;; EXPECT: true
(let [result (benchmark 10 (* 2 3))]
  (and (contains? result :min)
       (contains? result :max)
       (contains? result :mean)
       (contains? result :median)))

;; TEST: with-timeout completes
;; EXPECT: :done
(with-timeout 1000
  :done)

;; TEST: with-timeout exceeds
;; EXPECT: :timeout
(handle
  (with-timeout 1
    (do (range 1000000) :done))
  (TimeoutError [e k] :timeout))

;; TEST: elapsed-time in context
;; EXPECT: true
(>= (with-timer
      (+ 1 2)
      (elapsed-time))
    0)

;; TEST: measure-memory
;; EXPECT: true
(let [result (measure-memory (list 1 2 3 4 5))]
  (>= (get result :bytes) 0))

;; TEST: profile function calls
;; EXPECT-FINAL: true
(let [result (profile
               (map (lambda [x] (* x x)) (range 100)))]
  (contains? result :call-count))
