;; test_update_in.lisp - Tests for nested path update with function

;; Update-in applies function to value at nested path
(define update-in [coll] [path] [f]
  (match path
    ()       (f coll)
    (k)      (let [current (dict-get coll k nil)]
               (dict-set coll k (f current)))
    (k .. rest)
      (let [inner (dict-get coll k #{})]
        (dict-set coll k (update-in inner rest f)))))

;; TEST: update-in single level increment
;; EXPECT: #{"count" 2}
(update-in #{"count" 1} '("count") (lambda [x] (+ x 1)))

;; TEST: update-in two levels
;; EXPECT: #{"stats" #{"views" 101}}
(update-in #{"stats" #{"views" 100}} '("stats" "views") (lambda [x] (+ x 1)))

;; TEST: update-in three levels
;; EXPECT: #{"a" #{"b" #{"c" 20}}}
(update-in #{"a" #{"b" #{"c" 10}}} '("a" "b" "c") (lambda [x] (* x 2)))

;; TEST: update-in preserves other fields
;; EXPECT: #{"user" #{"name" "ALICE" "age" 30}}
(update-in #{"user" #{"name" "alice" "age" 30}} '("user" "name") str-upper)

;; TEST: update-in with nil (missing)
;; EXPECT: #{"new" #{"key" 1}}
(update-in #{} '("new" "key") (lambda [x] (if (nil? x) 1 (+ x 1))))

;; Counter nested in structure
(define counters #{"clicks" #{"button" 5 "link" 10}
                   "views" #{"page" 100}})

;; TEST: increment nested counter
;; EXPECT: #{"clicks" #{"button" 6 "link" 10} "views" #{"page" 100}}
(update-in counters '("clicks" "button") (lambda [x] (+ x 1)))

;; TEST: double nested value
;; EXPECT-FINAL: #{"clicks" #{"button" 5 "link" 10} "views" #{"page" 200}}
(update-in counters '("views" "page") (lambda [x] (* x 2)))
