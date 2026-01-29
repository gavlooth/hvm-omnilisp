;; test_defsuite.lisp - Tests for test suite definition

;; Defsuite groups related tests
;; TEST: basic suite
;; EXPECT: true
(do
  (defsuite math-tests
    (deftest add-test
      (assert-eq (+ 1 2) 3))
    (deftest mul-test
      (assert-eq (* 2 3) 6)))
  (suite? math-tests))

;; TEST: suite has tests
;; EXPECT: 2
(do
  (defsuite my-suite
    (deftest t1 (assert true))
    (deftest t2 (assert true)))
  (length (suite-tests my-suite)))

;; TEST: nested suites
;; EXPECT: true
(do
  (defsuite outer
    (defsuite inner
      (deftest t1 (assert true))))
  (suite? (first (suite-children outer))))

;; TEST: suite with setup
;; EXPECT: :setup-ran
(do
  (def state nil)
  (defsuite with-setup
    :setup (lambda [] (set! state :setup-ran))
    (deftest t1 (assert true)))
  (run-suite with-setup)
  state)

;; TEST: suite with teardown
;; EXPECT: :torn-down
(do
  (def state nil)
  (defsuite with-teardown
    :teardown (lambda [] (set! state :torn-down))
    (deftest t1 (assert true)))
  (run-suite with-teardown)
  state)

;; TEST: suite metadata
;; EXPECT: "Math operations"
(do
  (defsuite math-suite
    :description "Math operations"
    (deftest t1 (assert true)))
  (get (suite-meta math-suite) :description))

;; TEST: suite tags
;; EXPECT: (:unit :fast)
(do
  (defsuite tagged
    :tags (:unit :fast)
    (deftest t1 (assert true)))
  (get (suite-meta tagged) :tags))

;; TEST: suite before-each
;; EXPECT-FINAL: 3
(do
  (def count 0)
  (defsuite counted
    :before-each (lambda [] (set! count (+ count 1)))
    (deftest t1 (assert true))
    (deftest t2 (assert true))
    (deftest t3 (assert true)))
  (run-suite counted)
  count)
