;; test_format.lisp - Tests for format string formatting

;; Format with placeholders
;; TEST: format simple
;; EXPECT: "Hello World"
(format "Hello {}" "World")

;; TEST: format multiple
;; EXPECT: "1 + 2 = 3"
(format "{} + {} = {}" 1 2 3)

;; TEST: format indexed
;; EXPECT: "B A"
(format "{1} {0}" "A" "B")

;; TEST: format named
;; EXPECT: "Hello Alice"
(format "Hello {name}" :name "Alice")

;; TEST: format repeat
;; EXPECT: "hi hi hi"
(format "{0} {0} {0}" "hi")

;; Format with padding
;; TEST: format pad left
;; EXPECT: "  42"
(format "{:>4}" 42)

;; TEST: format pad right
;; EXPECT: "42  "
(format "{:<4}" 42)

;; TEST: format pad center
;; EXPECT: " 42 "
(format "{:^4}" 42)

;; TEST: format pad with char
;; EXPECT: "0042"
(format "{:0>4}" 42)

;; Format numbers
;; TEST: format decimal
;; EXPECT: "3.14"
(format "{:.2}" 3.14159)

;; TEST: format scientific
;; EXPECT: "1.23e+04"
(format "{:e}" 12345)

;; TEST: format hex
;; EXPECT: "ff"
(format "{:x}" 255)

;; TEST: format binary
;; EXPECT: "1010"
(format "{:b}" 10)

;; TEST: format with sign
;; EXPECT: "+42"
(format "{:+}" 42)

;; TEST: format thousand separator
;; EXPECT-FINAL: "1,234,567"
(format "{:,}" 1234567)
