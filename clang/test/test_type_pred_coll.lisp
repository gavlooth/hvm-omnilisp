;; test_type_pred_coll.lisp - Tests for collection predicates

;; TEST: array? with array
;; EXPECT: true
(array? [1 2 3])

;; TEST: array? with list
;; EXPECT: false
(array? '(1 2 3))

;; TEST: array? with empty array
;; EXPECT: true
(array? [])

;; TEST: dict? with dict
;; EXPECT: true
(dict? #{"a" 1})

;; TEST: dict? with empty dict
;; EXPECT: true
(dict? #{})

;; TEST: dict? with list
;; EXPECT: false
(dict? '())

;; TEST: set? with set
;; EXPECT: true
(set? #{1 2 3})

;; TEST: set? with dict
;; EXPECT: false
(set? #{"a" 1 "b" 2})

;; General collection predicate
(define coll? [x]
  (or (list? x) (array? x) (dict? x) (set? x)))

;; TEST: coll? list
;; EXPECT: true
(coll? '(1 2 3))

;; TEST: coll? array
;; EXPECT: true
(coll? [1 2 3])

;; TEST: coll? dict
;; EXPECT: true
(coll? #{"a" 1})

;; TEST: coll? scalar
;; EXPECT: false
(coll? 42)

;; Sequential? (ordered collections)
(define sequential? [x]
  (or (list? x) (array? x)))

;; TEST: sequential? list
;; EXPECT: true
(sequential? '(1 2 3))

;; TEST: sequential? array
;; EXPECT: true
(sequential? [1 2 3])

;; TEST: sequential? dict
;; EXPECT: false
(sequential? #{})

;; Associative? (keyed collections)
(define associative? [x]
  (or (dict? x) (array? x)))

;; TEST: associative? dict
;; EXPECT: true
(associative? #{"a" 1})

;; TEST: associative? list
;; EXPECT-FINAL: false
(associative? '(1 2 3))
