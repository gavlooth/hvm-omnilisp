;; test_datetime_now.lisp - Tests for datetime operations

;; Datetime-now returns current time
;; TEST: now returns datetime
;; EXPECT: true
(datetime? (datetime-now))

;; TEST: now has year
;; EXPECT: true
(> (datetime-year (datetime-now)) 2000)

;; TEST: now has month
;; EXPECT: true
(let [m (datetime-month (datetime-now))]
  (and (>= m 1) (<= m 12)))

;; TEST: now has day
;; EXPECT: true
(let [d (datetime-day (datetime-now))]
  (and (>= d 1) (<= d 31)))

;; Create datetime
;; TEST: create datetime
;; EXPECT: true
(datetime? (datetime 2024 1 15 10 30 0))

;; TEST: datetime year
;; EXPECT: 2024
(datetime-year (datetime 2024 1 15 10 30 0))

;; TEST: datetime month
;; EXPECT: 1
(datetime-month (datetime 2024 1 15 10 30 0))

;; TEST: datetime day
;; EXPECT: 15
(datetime-day (datetime 2024 1 15 10 30 0))

;; TEST: datetime hour
;; EXPECT: 10
(datetime-hour (datetime 2024 1 15 10 30 0))

;; TEST: datetime minute
;; EXPECT: 30
(datetime-minute (datetime 2024 1 15 10 30 0))

;; Timestamp
;; TEST: timestamp
;; EXPECT: true
(> (datetime-timestamp (datetime-now)) 0)

;; TEST: from timestamp
;; EXPECT: true
(datetime? (datetime-from-timestamp 1704067200))

;; TEST: timestamp roundtrip
;; EXPECT-FINAL: true
(let [ts (datetime-timestamp (datetime-now))]
  (= ts (datetime-timestamp (datetime-from-timestamp ts))))
