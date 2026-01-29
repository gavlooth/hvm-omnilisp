;; test_set_union.lisp - Tests for set union

;; Set union combines two sets
(define set-union [s1] [s2]
  (dict-merge s1 s2))

;; TEST: union of disjoint sets
;; EXPECT: #{1 true 2 true 3 true 4 true}
(set-union #{1 2} #{3 4})

;; TEST: union with overlap
;; EXPECT: #{1 true 2 true 3 true}
(set-union #{1 2} #{2 3})

;; TEST: union with empty set (left)
;; EXPECT: #{1 true 2 true}
(set-union #{} #{1 2})

;; TEST: union with empty set (right)
;; EXPECT: #{1 true 2 true}
(set-union #{1 2} #{})

;; TEST: union of empty sets
;; EXPECT: #{}
(set-union #{} #{})

;; TEST: union is idempotent with self
;; EXPECT: #{1 true 2 true 3 true}
(set-union #{1 2 3} #{1 2 3})

;; TEST: multiple unions
;; EXPECT: #{1 true 2 true 3 true 4 true 5 true 6 true}
(set-union (set-union #{1 2} #{3 4}) #{5 6})

;; TEST: union with symbols
;; EXPECT-FINAL: #{:a true :b true :c true :d true}
(set-union #{:a :b} #{:c :d})
