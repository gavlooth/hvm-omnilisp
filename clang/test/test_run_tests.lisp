;; test_run_tests.lisp - Tests for test runner

;; Run-tests executes test suites
;; TEST: run single test
;; EXPECT: :passed
(do
  (deftest simple (assert-eq 1 1))
  (get (run-test simple) :status))

;; TEST: run failing test
;; EXPECT: :failed
(do
  (deftest failing (assert-eq 1 2))
  (get (run-test failing) :status))

;; TEST: run suite returns results
;; EXPECT: true
(do
  (defsuite s
    (deftest t1 (assert true)))
  (map? (run-suite s)))

;; TEST: results include pass count
;; EXPECT: 2
(do
  (defsuite s
    (deftest t1 (assert true))
    (deftest t2 (assert true)))
  (get (run-suite s) :passed))

;; TEST: results include fail count
;; EXPECT: 1
(do
  (defsuite s
    (deftest t1 (assert true))
    (deftest t2 (assert false)))
  (get (run-suite s) :failed))

;; TEST: run with filter
;; EXPECT: 1
(do
  (defsuite s
    (deftest test-add (assert-eq (+ 1 1) 2))
    (deftest test-mul (assert-eq (* 2 2) 4)))
  (get (run-suite s :filter "add") :total))

;; TEST: run with tag filter
;; EXPECT: 1
(do
  (defsuite s
    (deftest ^:slow t1 (assert true))
    (deftest ^:fast t2 (assert true)))
  (get (run-suite s :tags [:fast]) :total))

;; TEST: run all suites
;; EXPECT: true
(do
  (defsuite s1 (deftest t1 (assert true)))
  (defsuite s2 (deftest t2 (assert true)))
  (let [results (run-all-suites)]
    (> (get results :total) 0)))

;; TEST: test timing
;; EXPECT: true
(do
  (deftest timed (assert true))
  (let [result (run-test timed)]
    (>= (get result :duration-ms) 0)))

;; TEST: verbose output
;; EXPECT-FINAL: :completed
(do
  (defsuite s (deftest t1 (assert true)))
  (run-suite s :verbose true)
  :completed)
