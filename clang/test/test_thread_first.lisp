;; test_thread_first.lisp - Tests for -> (thread-first) macro

;; Thread-first inserts value as first argument of each form
;; (-> x (f a) (g b)) = (g (f x a) b)

;; Simulate with explicit piping for now
(define thread-> [x] [f] (f x))

;; TEST: thread through increment
;; EXPECT: 6
(thread-> 5 (lambda [x] (+ x 1)))

;; TEST: thread through subtract first
;; EXPECT: 7
(thread-> 10 (lambda [x] (- x 3)))

;; Manual threading simulation
;; (-> 5 (+ 3) (* 2)) = (* (+ 5 3) 2) = 16
;; TEST: manual thread first chain
;; EXPECT: 16
(* (+ 5 3) 2)

;; (-> "hello" (str-upper)) = (str-upper "hello")
;; TEST: thread through str-upper
;; EXPECT: "HELLO"
(str-upper "hello")

;; Thread through dict operations
;; (-> #{} (dict-set "a" 1) (dict-set "b" 2))
;; TEST: thread through dict-set
;; EXPECT: #{"a" 1 "b" 2}
(dict-set (dict-set #{} "a" 1) "b" 2)

;; Thread through list operations
;; (-> '(1 2 3) (cons 0) reverse) = (reverse (cons 0 '(1 2 3)))
;; TEST: thread through cons and reverse
;; EXPECT: (1 2 3 0)
(reverse (cons 0 '(1 2 3)))

;; Complex threading
;; (-> 100
;;     (/ 10)     ; 10
;;     (+ 5)      ; 15
;;     (* 2))     ; 30
;; TEST: complex thread-first
;; EXPECT-FINAL: 30
(* (+ (/ 100 10) 5) 2)
