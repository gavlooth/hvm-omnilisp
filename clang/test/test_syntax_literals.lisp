;; test_syntax_literals.lisp - Tests for syntax-rules literals

;; Literals in syntax-rules are matched exactly
;; TEST: basic literal keyword
;; EXPECT: :matched
(do
  (define-syntax my-if
    :literals (then else)
    [(_ cond then t else e) `(if ,cond ,t ,e)])
  (my-if true then :matched else :not))

;; TEST: literal prevents binding
;; EXPECT: :yes
(do
  (define-syntax check-lit
    :literals (yes)
    [(_ yes) :yes]
    [(_ x) :no])
  (check-lit yes))

;; TEST: non-literal binds variable
;; EXPECT: 42
(do
  (define-syntax check-lit
    :literals (yes)
    [(_ yes) :literal]
    [(_ x) x])
  (check-lit 42))

;; TEST: multiple literals
;; EXPECT: (3 "hello")
(do
  (define-syntax config
    :literals (port host)
    [(_ port p host h) `(list ,p ,h)])
  (config port 3 host "hello"))

;; TEST: literal in nested position
;; EXPECT: :found
(do
  (define-syntax find-key
    :literals (key)
    [(_ (key v) ...) `(list ,@v)]
    [(_ _) :not-found])
  (if (list? (find-key (key 1) (key 2))) :found :error))

;; TEST: arrow as literal
;; EXPECT: (lambda [x] (+ x 1))
(do
  (define-syntax fn-arrow
    :literals (=>)
    [(_ args => body) `(lambda ,args ,body)])
  (fn-arrow [x] => (+ x 1)))

;; TEST: colon as literal
;; EXPECT-FINAL: #{"name" "test" "value" 42}
(do
  (define-syntax dict-lit
    :literals (:)
    [(_ (k : v) ...) `(dict ,@(interleave k v))])
  (dict-lit ("name" : "test") ("value" : 42)))
