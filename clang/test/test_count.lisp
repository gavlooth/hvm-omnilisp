;; test_count.lisp - Tests for count functions

;; Count counts elements matching predicate
(define count-if [pred] [lst]
  (length (filter pred lst)))

;; TEST: count-if evens
;; EXPECT: 3
(count-if (lambda [x] (= (% x 2) 0)) '(1 2 3 4 5 6))

;; TEST: count-if all
;; EXPECT: 5
(count-if (lambda [x] (> x 0)) '(1 2 3 4 5))

;; TEST: count-if none
;; EXPECT: 0
(count-if (lambda [x] (< x 0)) '(1 2 3 4 5))

;; TEST: count-if empty
;; EXPECT: 0
(count-if (lambda [x] true) '())

;; Count specific value
(define count [val] [lst]
  (count-if (lambda [x] (= x val)) lst))

;; TEST: count value
;; EXPECT: 3
(count 2 '(1 2 2 3 2 4))

;; TEST: count not found
;; EXPECT: 0
(count 99 '(1 2 3))

;; TEST: count in empty
;; EXPECT: 0
(count 1 '())

;; Count-not
(define count-not [pred] [lst]
  (count-if (lambda [x] (not (pred x))) lst))

;; TEST: count-not evens (count odds)
;; EXPECT: 3
(count-not (lambda [x] (= (% x 2) 0)) '(1 2 3 4 5 6))

;; Count with limit
(define count-if-max [pred] [lst] [max-count]
  (let helper [l lst] [n 0]
    (if (>= n max-count)
      n
      (match l
        ()       n
        (h .. t) (if (pred h)
                   (helper t (+ n 1))
                   (helper t n))))))

;; TEST: count-if-max
;; EXPECT: 3
(count-if-max (lambda [x] true) '(1 2 3 4 5 6 7 8 9 10) 3)

;; TEST: count-if-max not reached
;; EXPECT-FINAL: 2
(count-if-max (lambda [x] (= (% x 2) 0)) '(1 2 3 4) 5)
