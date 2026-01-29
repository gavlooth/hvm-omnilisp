;; test_dict_merge.lisp - Tests for dict-merge

;; Basic merge (right overwrites left on conflict)
;; TEST: merge non-overlapping
;; EXPECT: #{:a 1 :b 2 :c 3 :d 4}
(dict-merge #{:a 1 :b 2} #{:c 3 :d 4})

;; TEST: merge with overlap (right wins)
;; EXPECT: #{:a 1 :b 20 :c 3}
(dict-merge #{:a 1 :b 2} #{:b 20 :c 3})

;; TEST: merge with empty left
;; EXPECT: #{:a 1 :b 2}
(dict-merge #{} #{:a 1 :b 2})

;; TEST: merge with empty right
;; EXPECT: #{:a 1 :b 2}
(dict-merge #{:a 1 :b 2} #{})

;; TEST: merge both empty
;; EXPECT: #{}
(dict-merge #{} #{})

;; Merge multiple dicts
(define merge-all [dicts]
  (foldl dict-merge #{} dicts))

;; TEST: merge multiple dicts
;; EXPECT: #{:a 1 :b 2 :c 3 :d 4 :e 5}
(merge-all '(#{:a 1} #{:b 2 :c 3} #{:d 4 :e 5}))

;; Merge with function (merge-with)
(define merge-with [f] [d1] [d2]
  (foldl (lambda [acc] [entry]
           (match entry
             (k v) (dict-set acc k
                     (match (dict-get acc k)
                       nothing  v
                       old      (f old v)))))
         d1
         (dict-entries d2)))

;; TEST: merge-with addition
;; EXPECT-FINAL: #{:a 1 :b 22 :c 3}
(merge-with (lambda [a] [b] (+ a b)) #{:a 1 :b 2} #{:b 20 :c 3})
