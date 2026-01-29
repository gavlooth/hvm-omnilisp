;; test_list_find.lisp - Tests for find, find-index, index-of

;; Find first element matching predicate
(define find [pred] [lst]
  (match lst
    ()       nothing
    (h .. t) (if (pred h) h (find pred t))))

;; TEST: find existing element
;; EXPECT: 4
(find (lambda [x] (> x 3)) '(1 2 3 4 5))

;; TEST: find no match
;; EXPECT: nothing
(find (lambda [x] (> x 10)) '(1 2 3 4 5))

;; TEST: find first match only
;; EXPECT: 2
(find (lambda [x] (= (% x 2) 0)) '(1 2 3 4 5))

;; Find-index returns position of first match
(define find-index-helper [pred] [lst] [idx]
  (match lst
    ()       nothing
    (h .. t) (if (pred h) idx (find-index-helper pred t (+ idx 1)))))

(define find-index [pred] [lst]
  (find-index-helper pred lst 0))

;; TEST: find-index existing
;; EXPECT: 3
(find-index (lambda [x] (> x 3)) '(1 2 3 4 5))

;; TEST: find-index no match
;; EXPECT: nothing
(find-index (lambda [x] (> x 10)) '(1 2 3 4 5))

;; Index-of finds specific value
(define index-of [val] [lst]
  (find-index (lambda [x] (= x val)) lst))

;; TEST: index-of existing
;; EXPECT: 2
(index-of 3 '(1 2 3 4 5))

;; TEST: index-of not found
;; EXPECT-FINAL: nothing
(index-of 10 '(1 2 3 4 5))
