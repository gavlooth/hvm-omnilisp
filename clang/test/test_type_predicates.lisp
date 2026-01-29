;; test_type_predicates.lisp - Tests for type? predicates

;; TEST: int? true
;; EXPECT: 1
(int? 42)

;; TEST: int? false on list
;; EXPECT: 0
(int? (cons 1 nil))

;; TEST: list? true
;; EXPECT: 1
(list? (cons 1 nil))

;; TEST: list? true on nil
;; EXPECT: 1
(list? nil)

;; TEST: list? false on int
;; EXPECT: 0
(list? 42)
