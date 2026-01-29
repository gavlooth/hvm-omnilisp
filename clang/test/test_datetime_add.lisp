;; test_datetime_add.lisp - Tests for datetime arithmetic

;; Add days
;; TEST: add days
;; EXPECT: 20
(datetime-day (datetime-add (datetime 2024 1 15 0 0 0) 5 :days))

;; TEST: add days crosses month
;; EXPECT: 2
(datetime-month (datetime-add (datetime 2024 1 30 0 0 0) 5 :days))

;; TEST: add negative days
;; EXPECT: 10
(datetime-day (datetime-add (datetime 2024 1 15 0 0 0) -5 :days))

;; Add months
;; TEST: add months
;; EXPECT: 4
(datetime-month (datetime-add (datetime 2024 1 15 0 0 0) 3 :months))

;; TEST: add months crosses year
;; EXPECT: 2025
(datetime-year (datetime-add (datetime 2024 11 15 0 0 0) 3 :months))

;; Add years
;; TEST: add years
;; EXPECT: 2027
(datetime-year (datetime-add (datetime 2024 1 15 0 0 0) 3 :years))

;; Add hours
;; TEST: add hours
;; EXPECT: 15
(datetime-hour (datetime-add (datetime 2024 1 15 10 0 0) 5 :hours))

;; TEST: add hours crosses day
;; EXPECT: 16
(datetime-day (datetime-add (datetime 2024 1 15 22 0 0) 5 :hours))

;; Add minutes
;; TEST: add minutes
;; EXPECT: 45
(datetime-minute (datetime-add (datetime 2024 1 15 10 30 0) 15 :minutes))

;; Add seconds
;; TEST: add seconds
;; EXPECT: 30
(datetime-second (datetime-add (datetime 2024 1 15 10 30 0) 30 :seconds))

;; Subtract (add negative)
;; TEST: subtract days
;; EXPECT: 10
(datetime-day (datetime-add (datetime 2024 1 15 0 0 0) -5 :days))

;; Combined
;; TEST: add multiple units
;; EXPECT-FINAL: true
(let [dt (datetime 2024 1 15 10 30 0)]
  (let [result (datetime-add (datetime-add dt 1 :days) 2 :hours)]
    (and (= 16 (datetime-day result))
         (= 12 (datetime-hour result)))))
