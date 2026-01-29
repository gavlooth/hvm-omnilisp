;; test_set_subset.lisp - Tests for subset operations

;; Check if all keys in s1 are in s2
(define subset?-helper [s2] [keys]
  (match keys
    ()       true
    (k .. rest)
      (if (dict-has? s2 k)
        (subset?-helper s2 rest)
        false)))

(define subset? [s1] [s2]
  (subset?-helper s2 (dict-keys s1)))

;; TEST: subset - true case
;; EXPECT: true
(subset? #{1 2} #{1 2 3 4})

;; TEST: subset - equal sets
;; EXPECT: true
(subset? #{1 2 3} #{1 2 3})

;; TEST: subset - empty is subset of all
;; EXPECT: true
(subset? #{} #{1 2 3})

;; TEST: subset - empty is subset of empty
;; EXPECT: true
(subset? #{} #{})

;; TEST: subset - false case
;; EXPECT: false
(subset? #{1 2 5} #{1 2 3 4})

;; TEST: subset - larger not subset of smaller
;; EXPECT: false
(subset? #{1 2 3} #{1 2})

;; Superset is reverse
(define superset? [s1] [s2]
  (subset? s2 s1))

;; TEST: superset - true case
;; EXPECT: true
(superset? #{1 2 3 4} #{1 2})

;; TEST: superset - false case
;; EXPECT: false
(superset? #{1 2} #{1 2 3})

;; Proper subset (subset but not equal)
(define set-size [s] (length (dict-keys s)))

(define proper-subset? [s1] [s2]
  (and (subset? s1 s2)
       (not (= (set-size s1) (set-size s2)))))

;; TEST: proper subset - true
;; EXPECT: true
(proper-subset? #{1 2} #{1 2 3})

;; TEST: proper subset - equal sets (false)
;; EXPECT: false
(proper-subset? #{1 2 3} #{1 2 3})

;; Set equality
(define set-equal? [s1] [s2]
  (and (subset? s1 s2) (subset? s2 s1)))

;; TEST: set equality - true
;; EXPECT: true
(set-equal? #{1 2 3} #{3 2 1})

;; TEST: set equality - false
;; EXPECT-FINAL: false
(set-equal? #{1 2 3} #{1 2 4})
