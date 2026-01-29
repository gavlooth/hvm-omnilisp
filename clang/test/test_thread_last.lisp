;; test_thread_last.lisp - Tests for ->> (thread-last) macro

;; Thread-last inserts value as last argument of each form
;; (->> x (f a) (g b)) = (g b (f a x))

;; Simulate with explicit piping
(define thread->> [x] [f] (f x))

;; TEST: thread through identity
;; EXPECT: 42
(thread->> 42 (lambda [x] x))

;; Manual threading simulation
;; (->> '(1 2 3 4 5) (filter even?) (map double))
;; = (map double (filter even? '(1 2 3 4 5)))
;; = (map double '(2 4))
;; = '(4 8)

(define filter [pred] [lst]
  (match lst
    ()       '()
    (h .. t) (if (pred h)
               (cons h (filter pred t))
               (filter pred t))))

(define map [f] [lst]
  (match lst
    ()       '()
    (h .. t) (cons (f h) (map f t))))

;; TEST: thread-last through filter then map
;; EXPECT: (4 8)
(map (lambda [x] (* x 2)) (filter (lambda [x] (= (% x 2) 0)) '(1 2 3 4 5)))

;; (->> '(1 2 3) (map inc) (filter odd?))
;; = (filter odd? (map inc '(1 2 3)))
;; = (filter odd? '(2 3 4))
;; = '(3)
;; TEST: map then filter
;; EXPECT: (3)
(filter (lambda [x] (= (% x 2) 1)) (map (lambda [x] (+ x 1)) '(1 2 3)))

;; Thread through reducing
(define foldl [f] [acc] [lst]
  (match lst
    ()       acc
    (h .. t) (foldl f (f acc h) t)))

;; (->> '(1 2 3 4) (foldl + 0)) = (foldl + 0 '(1 2 3 4)) = 10
;; TEST: thread-last through fold
;; EXPECT: 10
(foldl (lambda [acc x] (+ acc x)) 0 '(1 2 3 4))

;; (->> '(1 2 3)
;;      (map square)       ; (1 4 9)
;;      (filter (> _ 3))   ; (4 9)
;;      (foldl + 0))       ; 13
;; TEST: complex thread-last
;; EXPECT-FINAL: 13
(foldl (lambda [acc x] (+ acc x)) 0 (filter (lambda [x] (> x 3)) (map (lambda [x] (* x x)) '(1 2 3))))
