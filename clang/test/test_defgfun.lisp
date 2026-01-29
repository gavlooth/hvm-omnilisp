;; test_defgfun.lisp - Tests for generic function definition

;; Define a generic function with defgfun
;; TEST: define generic function
;; EXPECT: true
(defgfun describe [x])
(fn? describe)

;; TEST: generic function exists
;; EXPECT: true
(defgfun area [shape])
(fn? area)

;; TEST: generic with multiple params
;; EXPECT: true
(defgfun combine [a] [b])
(fn? combine)

;; TEST: generic with return type
;; EXPECT: true
(defgfun stringify [x] {String})
(fn? stringify)

;; TEST: generic called without method
;; EXPECT: "no method"
(handle
  (defgfun no-impl [x])
  (no-impl 42)
  (NoMethodError [_ resume] (resume "no method")))

;; Generic dispatch placeholder
;; TEST: generic stores dispatch table
;; EXPECT-FINAL: true
(defgfun process [item])
(generic? process)
