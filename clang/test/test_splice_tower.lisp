;; test_splice_tower.lisp - Tests for splicing in tower

;; Splice combines code fragments in tower
;; TEST: basic splice in lift
;; EXPECT: 6
(run
  (let [a (lift 2)]
  (let [b (lift 3)]
    (lift (+ (splice a) (splice b) 1)))))

;; TEST: splice list
;; EXPECT: (1 2 3 4 5)
(run
  (let [xs (lift '(2 3 4))]
    (lift (list 1 (splice-list xs) 5))))

;; TEST: splice in quasiquote
;; EXPECT: (a 1 2 3 b)
(let [middle '(1 2 3)]
  `(a ,@middle b))

;; TEST: splice preserves structure
;; EXPECT: ((+ 1 2) (+ 3 4))
(run
  (let [e1 (lift '(+ 1 2))]
  (let [e2 (lift '(+ 3 4))]
    (lift (list (splice e1) (splice e2))))))

;; TEST: nested splice
;; EXPECT: 10
(run
  (let [inner (lift (lift 5))]
    (lift (* 2 (splice (run inner))))))

;; TEST: splice in function body
;; EXPECT: 15
(run
  (let [body (lift '(+ a b c))]
    (lift
      (let [a 3] (let [b 5] (let [c 7]
        (splice body)))))))

;; TEST: splice-map
;; EXPECT: (2 4 6)
(run
  (splice-map
    (lambda [x] (lift (* x 2)))
    '(1 2 3)))

;; TEST: conditional splice
;; EXPECT: (1 2 3)
(run
  (let [include true]
    (lift (list 1 (splice-when include (lift 2)) 3))))

;; TEST: splice removes level
;; EXPECT-FINAL: true
(let [staged (lift 42)]
  (= (code-level staged) (+ 1 (code-level (splice staged)))))
