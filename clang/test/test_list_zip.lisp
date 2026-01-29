;; test_list_zip.lisp - Tests for zip, zip-with, unzip

;; Zip two lists into pairs
(define zip [xs] [ys]
  (match xs
    () '()
    (x .. xt)
      (match ys
        () '()
        (y .. yt) (cons (cons x (cons y nil)) (zip xt yt)))))

;; TEST: zip equal length lists
;; EXPECT: ((1 a) (2 b) (3 c))
(zip '(1 2 3) '(a b c))

;; TEST: zip different length (truncates to shorter)
;; EXPECT: ((1 a) (2 b))
(zip '(1 2 3 4 5) '(a b))

;; TEST: zip with empty
;; EXPECT: ()
(zip '() '(1 2 3))

;; Zip-with applies function to pairs
(define zip-with [f] [xs] [ys]
  (match xs
    () '()
    (x .. xt)
      (match ys
        () '()
        (y .. yt) (cons (f x y) (zip-with f xt yt)))))

;; TEST: zip-with addition
;; EXPECT: (5 7 9)
(zip-with (lambda [a] [b] (+ a b)) '(1 2 3) '(4 5 6))

;; TEST: zip-with multiplication
;; EXPECT: (4 10 18)
(zip-with (lambda [a] [b] (* a b)) '(1 2 3) '(4 5 6))

;; Unzip splits list of pairs
(define unzip-helper [pairs] [xs] [ys]
  (match pairs
    () (cons (reverse xs) (cons (reverse ys) nil))
    ((a b) .. rest) (unzip-helper rest (cons a xs) (cons b ys))))

(define unzip [pairs] (unzip-helper pairs '() '()))

;; TEST: unzip list of pairs
;; EXPECT-FINAL: ((1 2 3) (a b c))
(unzip '((1 a) (2 b) (3 c)))
