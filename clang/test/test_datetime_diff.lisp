;; test_datetime_diff.lisp - Tests for datetime difference

;; Datetime-diff calculates time between dates
;; TEST: diff in days
;; EXPECT: 1
(datetime-diff
  (datetime 2024 1 1)
  (datetime 2024 1 2)
  :days)

;; TEST: diff in hours
;; EXPECT: 24
(datetime-diff
  (datetime 2024 1 1 0 0 0)
  (datetime 2024 1 2 0 0 0)
  :hours)

;; TEST: diff in minutes
;; EXPECT: 90
(datetime-diff
  (datetime 2024 1 1 0 0 0)
  (datetime 2024 1 1 1 30 0)
  :minutes)

;; TEST: diff in seconds
;; EXPECT: 3600
(datetime-diff
  (datetime 2024 1 1 0 0 0)
  (datetime 2024 1 1 1 0 0)
  :seconds)

;; TEST: diff in weeks
;; EXPECT: 2
(datetime-diff
  (datetime 2024 1 1)
  (datetime 2024 1 15)
  :weeks)

;; TEST: diff in months
;; EXPECT: 3
(datetime-diff
  (datetime 2024 1 15)
  (datetime 2024 4 15)
  :months)

;; TEST: diff in years
;; EXPECT: 2
(datetime-diff
  (datetime 2022 6 15)
  (datetime 2024 6 15)
  :years)

;; TEST: negative diff (past)
;; EXPECT: -5
(datetime-diff
  (datetime 2024 1 10)
  (datetime 2024 1 5)
  :days)

;; TEST: diff with time component
;; EXPECT: 36
(datetime-diff
  (datetime 2024 1 1 12 0 0)
  (datetime 2024 1 2 0 0 0)
  :hours)

;; TEST: diff between now
;; EXPECT-FINAL: true
(>=
  (datetime-diff (datetime-now) (datetime 2020 1 1) :days)
  0)
