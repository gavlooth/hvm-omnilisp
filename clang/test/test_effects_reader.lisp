;; test_effects_reader.lisp - Tests for Reader effect (ask)

;; Reader effect provides access to an immutable environment
;; Operation: Ask (read environment value)

;; TEST: ask for environment value
;; EXPECT: "production"
(handle
  (perform Ask :env)
  (Ask [key resume] (resume "production")))

;; TEST: multiple asks return same value
;; EXPECT: (42 42 42)
(handle
  (cons (perform Ask :config)
        (cons (perform Ask :config)
              (cons (perform Ask :config) nil)))
  (Ask [key resume] (resume 42)))

;; TEST: ask for different keys
;; EXPECT: ("localhost" 8080)
(handle
  (cons (perform Ask :host)
        (cons (perform Ask :port) nil))
  (Ask [key resume]
    (match key
      :host (resume "localhost")
      :port (resume 8080)
      _     (resume nil))))

;; Local override effect
;; TEST: local with different env
;; EXPECT: "testing"
(handle
  (perform Ask :env)
  (Ask [_ resume] (resume "testing")))

;; Computation using config
(define get-url []
  (handle
    (str-concat "http://"
                (perform Ask :host)
                ":"
                (int->str (perform Ask :port)))
    (Ask [key resume]
      (match key
        :host (resume "example.com")
        :port (resume 443)))))

;; TEST: build url from config
;; EXPECT: "http://example.com:443"
(get-url)

;; TEST: ask with default
;; EXPECT-FINAL: "default"
(handle
  (let [val (perform Ask :missing)]
    (if (nil? val) "default" val))
  (Ask [key resume] (resume nil)))
