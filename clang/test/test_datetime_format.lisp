;; test_datetime_format.lisp - Tests for datetime formatting

;; Datetime-format formats to string
;; TEST: format ISO
;; EXPECT: "2024-01-15T10:30:00"
(datetime-format (datetime 2024 1 15 10 30 0) "ISO")

;; TEST: format date only
;; EXPECT: "2024-01-15"
(datetime-format (datetime 2024 1 15 10 30 0) "YYYY-MM-DD")

;; TEST: format time only
;; EXPECT: "10:30:00"
(datetime-format (datetime 2024 1 15 10 30 0) "HH:mm:ss")

;; TEST: format custom
;; EXPECT: "15/01/2024"
(datetime-format (datetime 2024 1 15 10 30 0) "DD/MM/YYYY")

;; TEST: format with names
;; EXPECT: "January 15, 2024"
(datetime-format (datetime 2024 1 15 10 30 0) "MMMM DD, YYYY")

;; TEST: format weekday
;; EXPECT: "Monday"
(datetime-format (datetime 2024 1 15 10 30 0) "dddd")

;; TEST: format short weekday
;; EXPECT: "Mon"
(datetime-format (datetime 2024 1 15 10 30 0) "ddd")

;; TEST: format AM/PM
;; EXPECT: "10:30 AM"
(datetime-format (datetime 2024 1 15 10 30 0) "hh:mm A")

;; TEST: format PM
;; EXPECT: "10:30 PM"
(datetime-format (datetime 2024 1 15 22 30 0) "hh:mm A")

;; Parse datetime
;; TEST: parse ISO
;; EXPECT: 2024
(datetime-year (datetime-parse "2024-01-15T10:30:00" "ISO"))

;; TEST: parse custom
;; EXPECT: 15
(datetime-day (datetime-parse "15/01/2024" "DD/MM/YYYY"))

;; TEST: format then parse roundtrip
;; EXPECT-FINAL: true
(let [dt (datetime 2024 6 15 14 30 0)]
  (let [str (datetime-format dt "ISO")]
    (let [parsed (datetime-parse str "ISO")]
      (= (datetime-timestamp dt) (datetime-timestamp parsed)))))
