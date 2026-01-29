;; test_deftest.lisp - Tests for test framework

;; Deftest defines a test case
;; TEST: define test
;; EXPECT: true
(deftest add-test
  (assert-eq 4 (+ 2 2)))
(test? add-test)

;; TEST: run test passes
;; EXPECT: :pass
(run-test add-test)

;; TEST: run test fails
;; EXPECT: :fail
(deftest fail-test
  (assert-eq 5 (+ 2 2)))
(run-test fail-test)

;; Test with multiple assertions
;; TEST: multiple assertions
;; EXPECT: :pass
(deftest multi-assert
  (assert-eq 4 (+ 2 2))
  (assert-eq 6 (* 2 3))
  (assert (> 10 5)))
(run-test multi-assert)

;; Test with description
;; TEST: test description
;; EXPECT: "tests addition"
(deftest described-test "tests addition"
  (assert-eq 4 (+ 2 2)))
(test-description described-test)

;; Test with setup/teardown
;; TEST: test with setup
;; EXPECT: :pass
(deftest setup-test
  :setup (lambda [] #{"x" 10})
  :test (lambda [ctx] (assert-eq 10 (get ctx "x"))))
(run-test setup-test)

;; Test groups
;; TEST: test group
;; EXPECT: true
(deftest-group math-tests
  (deftest add-test (assert-eq 4 (+ 2 2)))
  (deftest sub-test (assert-eq 2 (- 5 3))))
(test-group? math-tests)

;; Run group
;; TEST: run test group
;; EXPECT: #{"passed" 2 "failed" 0}
(run-tests math-tests)

;; TEST: test result details
;; EXPECT-FINAL: true
(let [result (run-test add-test)]
  (= :pass result))
