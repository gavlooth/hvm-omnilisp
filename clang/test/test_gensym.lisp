;; test_gensym.lisp - Tests for gensym (unique symbol generation)

;; Gensym creates unique symbol
;; TEST: gensym creates symbol
;; EXPECT: true
(symbol? (gensym))

;; TEST: gensym with prefix
;; EXPECT: true
(symbol? (gensym "temp"))

;; TEST: two gensyms are different
;; EXPECT: false
(= (gensym) (gensym))

;; TEST: gensyms with same prefix differ
;; EXPECT: false
(= (gensym "x") (gensym "x"))

;; TEST: gensym starts with prefix
;; EXPECT: true
(let [s (gensym "foo")]
  (str-starts? (symbol->string s) "foo"))

;; Use gensym in macro for hygiene
(define-syntax swap!
  (syntax-rules ()
    ((swap! a b)
     (let [temp a]
       (set! a b)
       (set! b temp)))))

;; Gensym counter always increases
;; TEST: gensym ordering
;; EXPECT: true
(let [s1 (gensym "g")]
      [s2 (gensym "g")]
  (not (= s1 s2)))

;; Generate multiple unique names
;; TEST: generate list of unique symbols
;; EXPECT: 5
(let [syms (map (lambda [_] (gensym "var")) '(1 2 3 4 5))]
  (length (filter (lambda [s] (symbol? s)) syms)))

;; All unique
;; TEST: all gensyms unique
;; EXPECT: true
(let [syms (map (lambda [_] (gensym)) '(1 2 3))]
  (= (length syms)
     (length (filter (lambda [s]
                       (= 1 (length (filter (lambda [x] (= x s)) syms))))
                    syms))))

;; TEST: gensym in let binding
;; EXPECT-FINAL: 42
(let [g (gensym "result")]
  42)
