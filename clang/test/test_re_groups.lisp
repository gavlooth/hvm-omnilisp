;; test_re_groups.lisp - Tests for regex capture groups

;; Capture groups return matched substrings
;; TEST: single group
;; EXPECT: ("hello world" "hello")
(re-groups "hello world" "(\\w+) world")

;; TEST: multiple groups
;; EXPECT: ("hello world" "hello" "world")
(re-groups "hello world" "(\\w+) (\\w+)")

;; TEST: nested groups
;; EXPECT: ("abc123" "abc" "123" "12")
(re-groups "abc123" "(\\w+)(([0-9])[0-9]+)")

;; TEST: optional group matched
;; EXPECT: ("hello world" "hello" "world")
(re-groups "hello world" "(\\w+) ?(\\w+)?")

;; TEST: optional group not matched
;; EXPECT: ("hello" "hello" nil)
(re-groups "hello" "(\\w+) ?(\\w+)?")

;; Named groups
;; TEST: named group
;; EXPECT: #{"0" "hello world" "first" "hello" "second" "world"}
(re-groups-named "hello world" "(?<first>\\w+) (?<second>\\w+)")

;; TEST: access named group
;; EXPECT: "hello"
(let [m (re-groups-named "hello world" "(?<word>\\w+)")]
  (get m "word"))

;; Non-capturing groups
;; TEST: non-capturing
;; EXPECT: ("hello123" "123")
(re-groups "hello123" "(?:hello)(\\d+)")

;; Alternation with groups
;; TEST: alternation groups
;; EXPECT: ("cat" "cat" nil)
(re-groups "cat" "(cat)|(dog)")

;; TEST: alternation other branch
;; EXPECT: ("dog" nil "dog")
(re-groups "dog" "(cat)|(dog)")

;; Group count
;; TEST: count groups
;; EXPECT: 2
(length (tail (re-groups "ab" "(a)(b)")))

;; TEST: no groups
;; EXPECT-FINAL: ("hello")
(re-groups "hello" "hello")
