;; test_set_member.lisp - Tests for set membership

;; Set-member? checks if element is in set
(define set-member? [s] [elem]
  (dict-has? s elem))

;; TEST: member of set - found
;; EXPECT: true
(set-member? #{1 2 3} 2)

;; TEST: member of set - not found
;; EXPECT: false
(set-member? #{1 2 3} 99)

;; TEST: member of empty set
;; EXPECT: false
(set-member? #{} 1)

;; TEST: member check with symbol
;; EXPECT: true
(set-member? #{:a :b :c} :b)

;; TEST: member check with string
;; EXPECT: true
(set-member? #{"hello" "world"} "hello")

;; TEST: member check - wrong type (not found)
;; EXPECT: false
(set-member? #{1 2 3} "1")

;; Set-contains? is an alias
(define set-contains? [s] [elem] (set-member? s elem))

;; TEST: contains alias
;; EXPECT: true
(set-contains? #{:x :y :z} :y)

;; TEST: contains alias - not found
;; EXPECT-FINAL: false
(set-contains? #{:x :y :z} :w)
