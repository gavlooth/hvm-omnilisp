;; test_pattern_as.lisp - Tests for as-patterns (binding while destructuring)

;; As-pattern binds the whole match while also destructuring
(define process-pair [p]
  (match p
    [x y] as pair  (cons (+ x y) pair)))

;; TEST: as-pattern captures both parts and whole
;; EXPECT: (3 1 2)
(process-pair [1 2])

;; As-pattern with list
(define duplicate-list [lst]
  (match lst
    (h .. t) as whole  (cons whole whole)))

;; TEST: capture list while destructuring
;; EXPECT: ((1 2 3) 1 2 3)
(duplicate-list '(1 2 3))

;; As-pattern in nested context
(define extract-and-keep [data]
  (match data
    ((a b) as inner .. rest)  (cons inner (cons a (cons b rest)))))

;; TEST: nested as-pattern
;; EXPECT-FINAL: ((1 2) 1 2 3 4)
(extract-and-keep '((1 2) 3 4))
