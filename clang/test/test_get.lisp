;; test_get.lisp - Tests for generic get from any collection

;; Get works on lists (by index), dicts (by key), arrays (by index)
(define get [coll] [key] [default]
  (cond
    (list? coll)  (list-get coll key default)
    (dict? coll)  (dict-get coll key default)
    (array? coll) (arr-get coll key default)
    _             default))

;; Helper for list indexing
(define list-get [lst] [idx] [default]
  (if (< idx 0)
    default
    (match lst
      ()       default
      (h .. t) (if (= idx 0) h (list-get t (- idx 1) default)))))

;; TEST: get from list by index
;; EXPECT: 30
(get '(10 20 30 40) 2 nil)

;; TEST: get from list - out of bounds
;; EXPECT: :default
(get '(1 2 3) 10 :default)

;; TEST: get from dict by key
;; EXPECT: "value"
(get #{"a" "value" "b" "other"} "a" nil)

;; TEST: get from dict - missing key
;; EXPECT: :not-found
(get #{"a" 1} "z" :not-found)

;; TEST: get from array by index
;; EXPECT: 3
(get [1 2 3 4 5] 2 nil)

;; TEST: get from array - out of bounds
;; EXPECT: :oob
(get [1 2 3] 99 :oob)

;; TEST: get from empty list
;; EXPECT: :empty
(get '() 0 :empty)

;; TEST: get from empty dict
;; EXPECT: :empty
(get #{} "key" :empty)

;; TEST: get first element
;; EXPECT-FINAL: 100
(get '(100 200 300) 0 nil)
