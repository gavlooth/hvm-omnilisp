;; test_get_in.lisp - Tests for nested path access

;; Get-in traverses nested structures
(define get-in [coll] [path] [default]
  (match path
    ()       coll
    (k .. rest)
      (if (nil? coll)
        default
        (get-in (dict-get coll k nil) rest default))))

;; TEST: get-in single level
;; EXPECT: 1
(get-in #{"a" 1} '("a") nil)

;; TEST: get-in two levels
;; EXPECT: 42
(get-in #{"outer" #{"inner" 42}} '("outer" "inner") nil)

;; TEST: get-in three levels
;; EXPECT: "deep"
(get-in #{"a" #{"b" #{"c" "deep"}}} '("a" "b" "c") nil)

;; TEST: get-in missing path
;; EXPECT: :not-found
(get-in #{"a" #{"b" 1}} '("a" "x" "y") :not-found)

;; TEST: get-in missing intermediate
;; EXPECT: :missing
(get-in #{"a" 1} '("a" "b") :missing)

;; TEST: get-in empty path returns whole
;; EXPECT: #{"x" 1}
(get-in #{"x" 1} '() nil)

;; TEST: get-in from empty dict
;; EXPECT: :default
(get-in #{} '("any" "path") :default)

;; Nested data structure
(define user #{"name" "Alice"
               "address" #{"city" "NYC"
                          "zip" "10001"}
               "settings" #{"theme" "dark"
                           "notifications" #{"email" true
                                            "sms" false}}})

;; TEST: get user city
;; EXPECT: "NYC"
(get-in user '("address" "city") nil)

;; TEST: get notification setting
;; EXPECT: true
(get-in user '("settings" "notifications" "email") nil)

;; TEST: get missing deeply nested
;; EXPECT-FINAL: :no
(get-in user '("settings" "security" "2fa") :no)
