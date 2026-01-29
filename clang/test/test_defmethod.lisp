;; test_defmethod.lisp - Tests for method definition

;; Define methods for generic functions
(defgfun speak [animal])

;; TEST: define method for type
;; EXPECT: "woof"
(defmethod speak [animal {Dog}]
  "woof")
(speak (make-dog "Fido"))

;; TEST: define method for another type
;; EXPECT: "meow"
(defmethod speak [animal {Cat}]
  "meow")
(speak (make-cat "Whiskers"))

;; TEST: method dispatch correct type
;; EXPECT: "woof"
(let [pet (make-dog "Rex")]
  (speak pet))

;; Generic with multiple methods
(defgfun area [shape])

(defmethod area [shape {Circle}]
  (* 3.14159 (* (get shape :radius) (get shape :radius))))

(defmethod area [shape {Rectangle}]
  (* (get shape :width) (get shape :height)))

;; TEST: area circle
;; EXPECT: 78.53975
(area #{"type" "circle" "radius" 5})

;; TEST: area rectangle
;; EXPECT: 20
(area #{"type" "rectangle" "width" 4 "height" 5})

;; Default method
(defgfun to-string [x])

(defmethod to-string [x {Any}]
  "<unknown>")

(defmethod to-string [x {Int}]
  (int->str x))

;; TEST: to-string int
;; EXPECT: "42"
(to-string 42)

;; TEST: to-string fallback
;; EXPECT-FINAL: "<unknown>"
(to-string '(complex value))
