;; test_syntax_hygiene.lisp - Tests for macro hygiene

;; Hygienic macros prevent variable capture
;; TEST: macro doesn't capture outer variable
;; EXPECT: 10
(do
  (define-syntax swap
    [(_ a b)
     (let [temp (gensym)]
       `(let [,temp ,a]
          (set! ,a ,b)
          (set! ,b ,temp)))])
  (let [x 10] (let [y 20] (let [temp 5]
    (swap x y)
    temp))))

;; TEST: introduced bindings are fresh
;; EXPECT: 1
(do
  (define-syntax with-temp
    [(_ body) `(let [x 1] ,body)])
  (let [x 100]
    (with-temp x)))

;; TEST: gensym produces unique symbols
;; EXPECT: false
(do
  (let [a (gensym "x")]
  (let [b (gensym "x")]
    (= a b))))

;; TEST: syntax-parameterize
;; EXPECT: 5
(do
  (define-syntax-parameter it (lambda [stx] (error "it not in context")))
  (define-syntax aif
    [(_ test then else)
     `(let [result ,test]
        (syntax-parameterize ([it (lambda [stx] 'result)])
          (if result ,then ,else)))])
  (aif 5 it 0))

;; TEST: bound-identifier=?
;; EXPECT: true
(do
  (define-syntax check-same
    [(_ a b) (bound-identifier=? a b)])
  (let [x 1] (check-same x x)))

;; TEST: free-identifier=?
;; EXPECT: true
(do
  (let [+ (lambda [a b] (- a b))]
    (free-identifier=? '+ '+)))

;; TEST: datum->syntax preserves context
;; EXPECT: 42
(do
  (define-syntax make-binding
    [(_ name val)
     (datum->syntax #'name `(define ,#'name ,val))])
  (make-binding answer 42)
  answer)

;; TEST: unhygienic escape when needed
;; EXPECT-FINAL: :captured
(do
  (define-syntax capture
    [(_ var body)
     `(let [,var :captured] ,body)])
  (capture result result))
