;; test_datetime_parse.lisp - Tests for datetime parsing

;; Datetime-parse converts strings to datetime
;; TEST: parse ISO format
;; EXPECT: true
(datetime?
  (datetime-parse "2024-01-15" "yyyy-MM-dd"))

;; TEST: parse with time
;; EXPECT: 14
(datetime-hour
  (datetime-parse "2024-01-15 14:30:00" "yyyy-MM-dd HH:mm:ss"))

;; TEST: parse US format
;; EXPECT: 1
(datetime-month
  (datetime-parse "01/15/2024" "MM/dd/yyyy"))

;; TEST: parse European format
;; EXPECT: 15
(datetime-day
  (datetime-parse "15.01.2024" "dd.MM.yyyy"))

;; TEST: parse with timezone
;; EXPECT: true
(datetime?
  (datetime-parse "2024-01-15T14:30:00Z" "yyyy-MM-dd'T'HH:mm:ssX"))

;; TEST: parse month name
;; EXPECT: 1
(datetime-month
  (datetime-parse "January 15, 2024" "MMMM dd, yyyy"))

;; TEST: parse abbreviated month
;; EXPECT: 3
(datetime-month
  (datetime-parse "Mar 15, 2024" "MMM dd, yyyy"))

;; TEST: parse 12-hour time
;; EXPECT: 14
(datetime-hour
  (datetime-parse "2:30 PM" "h:mm a"))

;; TEST: parse with milliseconds
;; EXPECT: 123
(datetime-millis
  (datetime-parse "14:30:00.123" "HH:mm:ss.SSS"))

;; TEST: try-parse invalid
;; EXPECT: nil
(datetime-try-parse "not a date" "yyyy-MM-dd")

;; TEST: parse or default
;; EXPECT: true
(datetime?
  (datetime-parse-or "invalid" "yyyy-MM-dd" (datetime-now)))

;; TEST: roundtrip format/parse
;; EXPECT-FINAL: true
(let [dt (datetime 2024 6 15 10 30 0)]
(let [str (datetime-format dt "yyyy-MM-dd HH:mm:ss")]
(let [parsed (datetime-parse str "yyyy-MM-dd HH:mm:ss")]
  (= dt parsed))))
