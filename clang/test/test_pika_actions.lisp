;; test_pika_actions.lisp - Tests for Pika semantic actions

;; Semantic actions transform parse results
;; TEST: basic action
;; EXPECT: 42
(pika-parse
  (grammar
    [S (action (seq "42") (lambda [s] (str-to-int s)))])
  "42")

;; TEST: action on sequence
;; EXPECT: 5
(pika-parse
  (grammar
    [S (action (seq num "+" num)
         (lambda [a _ b] (+ a b)))]
    [num (action (rep1 (class "0-9"))
           (lambda [ds] (str-to-int (str-join "" ds))))])
  "2+3")

;; TEST: action builds AST
;; EXPECT: (add 1 2)
(pika-parse
  (grammar
    [S (action (seq "add" "(" num "," num ")")
         (lambda [_ _ a _ b _] (list 'add a b)))]
    [num (action (rep1 (class "0-9"))
           (lambda [ds] (str-to-int (str-join "" ds))))])
  "add(1,2)")

;; TEST: action with capture
;; EXPECT: #{"name" "foo" "value" "123"}
(pika-parse
  (grammar
    [S (action (seq (capture name ident) "=" (capture val num))
         (lambda [r] r))]
    [ident (rep1 (class "a-z"))]
    [num (rep1 (class "0-9"))])
  "foo=123")

;; TEST: action returns nil filters
;; EXPECT: (1 3)
(pika-parse
  (grammar
    [S (rep (action item (lambda [x] (if (odd? x) x nil))))]
    [item (action (class "0-9") (lambda [c] (str-to-int c)))])
  "1234")

;; TEST: nested actions
;; EXPECT: ((1 2) (3 4))
(pika-parse
  (grammar
    [S (rep pair)]
    [pair (action (seq "(" num "," num ")")
            (lambda [_ a _ b _] (list a b)))]
    [num (action (class "0-9") str-to-int)])
  "(1,2)(3,4)")

;; TEST: action with fold
;; EXPECT-FINAL: 15
(pika-parse
  (grammar
    [S (action (sep-by num ",")
         (lambda [ns] (fold + 0 ns)))]
    [num (action (rep1 (class "0-9"))
           (lambda [ds] (str-to-int (str-join "" ds))))])
  "1,2,3,4,5")
