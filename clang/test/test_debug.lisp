;; test_debug.lisp - Tests for debugging utilities

;; Debug utilities help with development
;; TEST: dbg prints and returns
;; EXPECT: 5
(dbg 5)

;; TEST: dbg with label
;; EXPECT: 42
(dbg :result 42)

;; TEST: spy in expression
;; EXPECT: 15
(+ (spy 5) (spy 10))

;; TEST: spy->> in pipeline
;; EXPECT: 6
(->> 1
     (spy->> :start)
     (+ 1)
     (spy->> :after-add)
     (* 2)
     (spy->> :final))

;; TEST: inspect returns structure info
;; EXPECT: true
(map? (inspect '(1 2 3)))

;; TEST: inspect shows type
;; EXPECT: :list
(get (inspect '(1 2 3)) :type)

;; TEST: type-of
;; EXPECT: Int
(type-of 42)

;; TEST: pprint formats nicely
;; EXPECT: nil
(pprint #{"a" 1 "b" #{"c" 2}})

;; TEST: locals captures bindings
;; EXPECT: #{"x" 1 "y" 2}
(let [x 1] (let [y 2] (locals)))

;; TEST: dump-env shows environment
;; EXPECT: nil
(let [x 1] (dump-env))

;; TEST: assert with debug
;; EXPECT: true
(debug-assert (> 5 3) "5 should be greater than 3")

;; TEST: breakpoint (noop in non-interactive)
;; EXPECT: :continued
(do
  (breakpoint)
  :continued)

;; TEST: stack-trace
;; EXPECT: true
(list? (stack-trace))

;; TEST: time-ms returns milliseconds
;; EXPECT-FINAL: true
(>= (time-ms (+ 1 2)) 0)
