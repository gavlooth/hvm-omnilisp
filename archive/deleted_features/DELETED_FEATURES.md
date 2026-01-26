# Deleted Features Archive

These features were implemented but removed. Some were incorrectly removed - see status for each.

**Note:** Features marked "IN SPEC" have been re-created with correct syntax.

---

## 1. Macro System (`lib/macro.hvm4`) → RESTORED as `lib/syntax.hvm4`

**Status:** ~~NOT IN SPEC~~ **CORRECTION: IN SPEC as `(define [syntax ...])`**

The correct form is `(define [syntax name] ...)` (hygienic macros), NOT `(defmacro ...)`.

See: `docs/UNDOCUMENTED_FEATURES.md`, `TODO.md`, `README.md`

**Re-created as:** `lib/syntax.hvm4`

### Correct Syntax (from spec)

```lisp
;; Simple hygienic macro
(define [syntax when]
  [(when test body ...)
   (if test (do body ...) nothing)])

;; Macro with literal keywords
(define [syntax case]
  [literals else =>]
  [(case val (else result))
   result]
  [(case val (pattern => result) rest ...)
   (if (= val pattern) result (case val rest ...))])

;; Thread-first macro
(define [syntax ->]
  [(-> x) x]
  [(-> x (f args ...) rest ...)
   (-> (f x args ...) rest ...)]
  [(-> x f rest ...)
   (-> (f x) rest ...)])

;; Usage
(when (> x 0)
  (print "positive")
  (do-something))

(-> data
    (transform)
    (filter valid?)
    (process))
```

### Pattern Syntax
- `name` - Pattern variable (binds to expression)
- `name ...` - Ellipsis (zero or more)
- `[literals kw1 kw2]` - Keywords that match literally

### Functions Implemented (in lib/syntax.hvm4)
- `@omni_define_syntax` - Define a syntax macro
- `@omni_syntax_expand` - Expand macro application
- `@omni_macroexpand` - Expand one level
- `@omni_macroexpand_all` - Full recursive expansion
- `@omni_gensym` - Unique symbol generation
- `@omni_syntax_match` - Pattern matching
- `@omni_syntax_instantiate` - Template instantiation

---

## 2. Module System (`lib/module.hvm4`)

**Status:** NOT IN SPEC - No module/namespace system defined.

### Syntax (as implemented)

```lisp
;; Define a module
(module my-module
  (export add subtract)

  (define add [x] [y] (+ x y))
  (define subtract [x] [y] (- x y))
  (define internal-helper [x] (* x 2)))  ; not exported

;; Import from module
(import my-module)           ; import all exports
(import my-module [add])     ; import specific
(import my-module :as m)     ; qualified import

;; Use qualified names
(my-module.add 1 2)
(m.add 1 2)

;; Require (load and import)
(require "path/to/module.ol")
```

### Functions Implemented
- `@omni_module` - Define module
- `@omni_import` - Import from module
- `@omni_export` - Mark symbols for export
- `@omni_require` - Load and import
- `@omni_module_registry` - Global module table

---

## 3. String Operations (`lib/string.hvm4`)

**Status:** NOT IN SPEC - No string manipulation functions defined.

### Syntax (as implemented)

```lisp
;; Concatenation
(str-concat "hello" " " "world")  ; => "hello world"
(str "a" "b" "c")                 ; => "abc"

;; Split and join
(str-split "a,b,c" ",")           ; => ["a" "b" "c"]
(str-join ["a" "b" "c"] ",")      ; => "a,b,c"

;; Case conversion
(str-upper "hello")               ; => "HELLO"
(str-lower "HELLO")               ; => "hello"
(str-capitalize "hello world")    ; => "Hello world"

;; Trimming
(str-trim "  hello  ")            ; => "hello"
(str-trim-left "  hello")         ; => "hello"
(str-trim-right "hello  ")        ; => "hello"

;; Substring
(str-substring "hello" 1 4)       ; => "ell"
(str-take "hello" 3)              ; => "hel"
(str-drop "hello" 2)              ; => "llo"

;; Search
(str-index-of "hello" "l")        ; => 2
(str-last-index-of "hello" "l")   ; => 3
(str-contains? "hello" "ell")     ; => true

;; Replace
(str-replace "hello" "l" "L")     ; => "heLLo"
(str-replace-first "hello" "l" "L") ; => "heLlo"

;; Predicates
(str-starts-with? "hello" "he")   ; => true
(str-ends-with? "hello" "lo")     ; => true
(str-blank? "   ")                ; => true
(str-empty? "")                   ; => true

;; Length and chars
(str-length "hello")              ; => 5
(str-chars "hello")               ; => [#\h #\e #\l #\l #\o]
(str-from-chars [#\h #\i])        ; => "hi"

;; Repeat and pad
(str-repeat "ab" 3)               ; => "ababab"
(str-pad-left "42" 5 "0")         ; => "00042"
(str-pad-right "hi" 5 ".")        ; => "hi..."

;; Reverse
(str-reverse "hello")             ; => "olleh"
```

### Functions Implemented
- `@omni_str_concat`, `@omni_str`
- `@omni_str_split`, `@omni_str_join`
- `@omni_str_upper`, `@omni_str_lower`, `@omni_str_capitalize`
- `@omni_str_trim`, `@omni_str_trim_left`, `@omni_str_trim_right`
- `@omni_str_substring`, `@omni_str_take`, `@omni_str_drop`
- `@omni_str_index_of`, `@omni_str_last_index_of`, `@omni_str_contains`
- `@omni_str_replace`, `@omni_str_replace_first`
- `@omni_str_starts_with`, `@omni_str_ends_with`
- `@omni_str_blank`, `@omni_str_empty`
- `@omni_str_length`, `@omni_str_chars`, `@omni_str_from_chars`
- `@omni_str_repeat`, `@omni_str_pad_left`, `@omni_str_pad_right`
- `@omni_str_reverse`

---

## 4. Loop/Recur (`lib/loop.hvm4`)

**Status:** NOT IN SPEC - Only `match` and `if` for control flow.

### Syntax (as implemented)

```lisp
;; Basic loop with recur
(loop [i 0 sum 0]
  (if (>= i 10)
    sum
    (recur (+ i 1) (+ sum i))))
;; => 45

;; Loop with multiple bindings
(loop [x 1 y 1 n 10]
  (if (= n 0)
    x
    (recur y (+ x y) (- n 1))))
;; => Fibonacci(10)

;; Recur must be in tail position
(loop [xs '(1 2 3) acc '()]
  (match xs
    [() (reverse acc)]
    [[h .. t] (recur t (cons (* h 2) acc))]))
```

### Functions Implemented
- `@omni_loop` - Loop construct with bindings
- `@omni_recur` - Tail-recursive jump
- `@omni_loop_eval` - Loop body evaluator

---

## 5. Pipe & Function Utilities (`lib/functions.hvm4`)

**Status:** NOT IN SPEC

### Syntax (as implemented)

```lisp
;; Thread-first (-> x f1 f2 f3)
;; x becomes first arg to each function
(-> 5
    (+ 3)      ; (+ 5 3) = 8
    (* 2)      ; (* 8 2) = 16
    (- 1))     ; (- 16 1) = 15

;; Thread-last (->> x f1 f2 f3)
;; x becomes last arg to each function
(->> [1 2 3 4 5]
     (filter odd?)     ; (filter odd? [1 2 3 4 5])
     (map square)      ; (map square [1 3 5])
     (reduce +))       ; (reduce + [1 9 25])

;; Compose (right to left)
(define add1-then-double (compose double add1))
(add1-then-double 5)   ; => 12 (double (add1 5))

;; Pipe (left to right)
(define add1-then-double (pipe add1 double))
(add1-then-double 5)   ; => 12 (double (add1 5))

;; Partial application
(define add5 (partial + 5))
(add5 3)               ; => 8

;; Currying
(define cadd (curry + 3))
((cadd 1) 2)           ; => 6

;; Flip arguments
(define sub-flipped (flip -))
(sub-flipped 3 10)     ; => 7 (10 - 3)

;; Identity
(identity 42)          ; => 42

;; Constantly
(define always-5 (constantly 5))
(always-5 'anything)   ; => 5

;; Complement (negate predicate)
(define not-empty? (complement empty?))
(not-empty? [1 2 3])   ; => true

;; Juxtaposition
(define stats (juxt min max sum))
(stats [1 2 3 4 5])    ; => [1 5 15]

;; Memoize
(define fib-memo (memoize fib))
(fib-memo 100)         ; fast on repeated calls
```

### Functions Implemented
- `@omni_thread_first` (`->`)
- `@omni_thread_last` (`->>`)
- `@omni_compose`
- `@omni_pipe`
- `@omni_partial`
- `@omni_curry`
- `@omni_flip`
- `@omni_identity`
- `@omni_constantly`
- `@omni_complement`
- `@omni_juxt`
- `@omni_memoize`

---

## 6. Conditional Forms (`lib/conditionals.hvm4`)

**Status:** NOT IN SPEC - Only `if` and `match` are specified.

### Syntax (as implemented)

```lisp
;; when - single branch (no else)
(when (> x 0)
  (print "positive")
  (do-something))
;; Returns nothing if condition false

;; unless - negated when
(unless (empty? xs)
  (process xs))

;; cond - multi-way conditional
(cond
  [(< x 0) "negative"]
  [(= x 0) "zero"]
  [(> x 0) "positive"])

;; cond with else
(cond
  [(< x 0) "negative"]
  [:else "non-negative"])

;; case - value dispatch
(case color
  [:red "stop"]
  [:yellow "slow"]
  [:green "go"]
  [:else "unknown"])

;; case with multiple values
(case day
  [(:saturday :sunday) "weekend"]
  [(:monday :tuesday :wednesday :thursday :friday) "weekday"])

;; if-let - conditional binding
(if-let [x (get-optional-value)]
  (use x)
  (handle-nothing))

;; when-let - when with binding
(when-let [x (find-item)]
  (process x))

;; if-some - Option type check
(if-some [val opt]
  (use val)
  (default))
```

### Functions Implemented
- `@omni_when`
- `@omni_unless`
- `@omni_cond`
- `@omni_case`
- `@omni_if_let`
- `@omni_when_let`
- `@omni_if_some`

---

## 7. Math Library (`lib/math.hvm4`)

**Status:** NOT IN SPEC

### Syntax (as implemented)

```lisp
;; Basic operations
(abs -5)              ; => 5
(min 3 1 4 1 5)       ; => 1
(max 3 1 4 1 5)       ; => 5
(clamp 15 0 10)       ; => 10

;; Rounding
(floor 3.7)           ; => 3
(ceil 3.2)            ; => 4
(round 3.5)           ; => 4
(trunc 3.9)           ; => 3

;; Powers and roots
(sqrt 16)             ; => 4.0
(cbrt 27)             ; => 3.0
(pow 2 10)            ; => 1024
(exp 1)               ; => 2.718...
(log 10)              ; => 2.302... (natural log)
(log10 100)           ; => 2.0
(log2 8)              ; => 3.0

;; Trigonometry (radians)
(sin 0)               ; => 0.0
(cos 0)               ; => 1.0
(tan 0)               ; => 0.0
(asin 0)              ; => 0.0
(acos 1)              ; => 0.0
(atan 0)              ; => 0.0
(atan2 1 1)           ; => 0.785... (pi/4)

;; Hyperbolic
(sinh 0)              ; => 0.0
(cosh 0)              ; => 1.0
(tanh 0)              ; => 0.0

;; Degrees/radians conversion
(deg->rad 180)        ; => 3.14159...
(rad->deg 3.14159)    ; => 180.0

;; Integer operations
(gcd 12 18)           ; => 6
(lcm 4 6)             ; => 12
(mod 17 5)            ; => 2
(quot 17 5)           ; => 3
(rem 17 5)            ; => 2
(divmod 17 5)         ; => [3 2]

;; Predicates
(even? 4)             ; => true
(odd? 3)              ; => true
(positive? 5)         ; => true
(negative? -5)        ; => true
(zero? 0)             ; => true
(nan? (/ 0.0 0.0))    ; => true
(infinite? (/ 1 0))   ; => true

;; Random
(random)              ; => 0.0-1.0 float
(random-int 100)      ; => 0-99 integer
(random-range 10 20)  ; => 10-19 integer

;; Constants
pi                    ; => 3.14159...
e                     ; => 2.71828...
tau                   ; => 6.28318... (2*pi)
```

### Functions Implemented
- `@omni_abs`, `@omni_min`, `@omni_max`, `@omni_clamp`
- `@omni_floor`, `@omni_ceil`, `@omni_round`, `@omni_trunc`
- `@omni_sqrt`, `@omni_cbrt`, `@omni_pow`, `@omni_exp`
- `@omni_log`, `@omni_log10`, `@omni_log2`
- `@omni_sin`, `@omni_cos`, `@omni_tan`
- `@omni_asin`, `@omni_acos`, `@omni_atan`, `@omni_atan2`
- `@omni_sinh`, `@omni_cosh`, `@omni_tanh`
- `@omni_deg_to_rad`, `@omni_rad_to_deg`
- `@omni_gcd`, `@omni_lcm`, `@omni_mod`, `@omni_quot`, `@omni_rem`, `@omni_divmod`
- `@omni_even`, `@omni_odd`, `@omni_positive`, `@omni_negative`, `@omni_zero`
- `@omni_nan`, `@omni_infinite`
- `@omni_random`, `@omni_random_int`, `@omni_random_range`

---

## 8. I/O System (`lib/io.hvm4`)

**Status:** NOT IN SPEC

### Syntax (as implemented)

```lisp
;; Console output
(print "hello")           ; no newline
(println "hello")         ; with newline
(prn value)               ; print with quotes for strings
(printf "x = %d" x)       ; formatted output

;; Console input
(read-line)               ; => string from stdin
(read-char)               ; => single character

;; File operations
(file-read "path.txt")            ; => string contents
(file-read-lines "path.txt")      ; => list of lines
(file-read-bytes "path.bin")      ; => byte array

(file-write "path.txt" "content") ; overwrite
(file-append "path.txt" "more")   ; append
(file-write-lines "path.txt" lines)
(file-write-bytes "path.bin" bytes)

;; File predicates
(file-exists? "path.txt")         ; => true/false
(file-readable? "path.txt")
(file-writable? "path.txt")
(file-directory? "path")
(file-regular? "path.txt")

;; File metadata
(file-size "path.txt")            ; => bytes
(file-modified "path.txt")        ; => timestamp
(file-created "path.txt")         ; => timestamp

;; File operations
(file-delete "path.txt")
(file-rename "old.txt" "new.txt")
(file-copy "src.txt" "dst.txt")
(file-move "src.txt" "dst.txt")

;; Directory operations
(dir-list "path")                 ; => list of names
(dir-list-recursive "path")       ; => all descendants
(dir-create "path")
(dir-create-parents "a/b/c")      ; mkdir -p
(dir-delete "path")               ; must be empty
(dir-delete-recursive "path")     ; rm -rf

;; Path operations
(path-join "a" "b" "c")           ; => "a/b/c"
(path-parent "a/b/c")             ; => "a/b"
(path-filename "a/b/c.txt")       ; => "c.txt"
(path-extension "file.txt")       ; => "txt"
(path-stem "file.txt")            ; => "file"
(path-absolute "rel/path")        ; => "/abs/rel/path"
(path-relative "/a/b" "/a")       ; => "b"

;; Standard streams
stdin                             ; standard input handle
stdout                            ; standard output handle
stderr                            ; standard error handle

;; With-open (auto-close)
(with-open [f (file-open "path.txt" :read)]
  (read-line f))
```

### Functions Implemented
- `@omni_print`, `@omni_println`, `@omni_prn`, `@omni_printf`
- `@omni_read_line`, `@omni_read_char`
- `@omni_file_read`, `@omni_file_read_lines`, `@omni_file_read_bytes`
- `@omni_file_write`, `@omni_file_append`, `@omni_file_write_lines`, `@omni_file_write_bytes`
- `@omni_file_exists`, `@omni_file_readable`, `@omni_file_writable`
- `@omni_file_directory`, `@omni_file_regular`
- `@omni_file_size`, `@omni_file_modified`, `@omni_file_created`
- `@omni_file_delete`, `@omni_file_rename`, `@omni_file_copy`, `@omni_file_move`
- `@omni_dir_list`, `@omni_dir_list_recursive`
- `@omni_dir_create`, `@omni_dir_create_parents`
- `@omni_dir_delete`, `@omni_dir_delete_recursive`
- `@omni_path_join`, `@omni_path_parent`, `@omni_path_filename`
- `@omni_path_extension`, `@omni_path_stem`
- `@omni_path_absolute`, `@omni_path_relative`

---

## 9. JSON (`lib/json.hvm4`)

**Status:** NOT IN SPEC

### Syntax (as implemented)

```lisp
;; Parse JSON string to OmniLisp value
(json-parse "{\"name\":\"Alice\",\"age\":30}")
;; => #{:name "Alice" :age 30}

(json-parse "[1, 2, 3]")
;; => [1 2 3]

;; Convert OmniLisp value to JSON string
(json-stringify #{:name "Bob" :active true})
;; => "{\"name\":\"Bob\",\"active\":true}"

;; Pretty print
(json-stringify value :pretty true)
;; => formatted with indentation

(json-stringify value :indent 4)
;; => 4-space indentation

;; Access JSON paths
(json-get obj "user.address.city")
;; => nested field access

(json-set obj "user.name" "Alice")
;; => new object with updated field

;; JSON predicates
(json-object? value)
(json-array? value)
(json-string? value)
(json-number? value)
(json-boolean? value)
(json-null? value)

;; Merge objects
(json-merge obj1 obj2)
;; => combined object (obj2 wins conflicts)

;; Select keys
(json-select obj [:name :age])
;; => object with only those keys
```

### Functions Implemented
- `@omni_json_parse`
- `@omni_json_stringify`
- `@omni_json_get`
- `@omni_json_set`
- `@omni_json_object`, `@omni_json_array`
- `@omni_json_string`, `@omni_json_number`
- `@omni_json_boolean`, `@omni_json_null`
- `@omni_json_merge`
- `@omni_json_select`

---

## 10. DateTime (`lib/datetime.hvm4`)

**Status:** NOT IN SPEC

### Syntax (as implemented)

```lisp
;; Current time
(now)                     ; => DateTime object
(now-utc)                 ; => UTC DateTime
(now-millis)              ; => Unix timestamp ms

;; Create dates
(date 2024 1 15)          ; => Date (year month day)
(time 14 30 0)            ; => Time (hour min sec)
(datetime 2024 1 15 14 30 0)  ; => DateTime

;; From timestamp
(from-timestamp 1705312200000)
(from-iso "2024-01-15T14:30:00Z")

;; Extract components
(year dt)                 ; => 2024
(month dt)                ; => 1
(day dt)                  ; => 15
(hour dt)                 ; => 14
(minute dt)               ; => 30
(second dt)               ; => 0
(millisecond dt)          ; => 0
(day-of-week dt)          ; => :monday
(day-of-year dt)          ; => 15

;; Arithmetic
(add-days dt 5)
(add-months dt 2)
(add-years dt 1)
(add-hours dt 3)
(add-minutes dt 30)
(add-seconds dt 45)

(subtract-days dt 5)
;; ... similar for all units

;; Difference
(diff-days dt1 dt2)       ; => integer
(diff-hours dt1 dt2)
(diff-minutes dt1 dt2)
(diff-seconds dt1 dt2)

;; Comparison
(before? dt1 dt2)
(after? dt1 dt2)
(same-day? dt1 dt2)

;; Formatting
(format-date dt "yyyy-MM-dd")
(format-date dt "MM/dd/yyyy HH:mm:ss")
(to-iso dt)               ; => "2024-01-15T14:30:00Z"

;; Parsing
(parse-date "2024-01-15" "yyyy-MM-dd")
(parse-date "01/15/2024" "MM/dd/yyyy")

;; Timezones
(to-timezone dt "America/New_York")
(to-utc dt)
(timezone-offset dt)      ; => hours from UTC

;; Ranges
(date-range start end)    ; => lazy seq of dates
(date-range start end :step 7)  ; weekly
```

### Functions Implemented
- `@omni_now`, `@omni_now_utc`, `@omni_now_millis`
- `@omni_date`, `@omni_time`, `@omni_datetime`
- `@omni_from_timestamp`, `@omni_from_iso`
- `@omni_year`, `@omni_month`, `@omni_day`, `@omni_hour`, `@omni_minute`, `@omni_second`
- `@omni_millisecond`, `@omni_day_of_week`, `@omni_day_of_year`
- `@omni_add_days`, `@omni_add_months`, `@omni_add_years`
- `@omni_add_hours`, `@omni_add_minutes`, `@omni_add_seconds`
- `@omni_diff_days`, `@omni_diff_hours`, `@omni_diff_minutes`, `@omni_diff_seconds`
- `@omni_before`, `@omni_after`, `@omni_same_day`
- `@omni_format_date`, `@omni_to_iso`
- `@omni_parse_date`
- `@omni_to_timezone`, `@omni_to_utc`, `@omni_timezone_offset`
- `@omni_date_range`

---

## 11. Networking (`lib/networking.hvm4`)

**Status:** NOT IN SPEC

### Syntax (as implemented)

```lisp
;; TCP Client
(tcp-connect "localhost" 8080)    ; => Handle
(tcp-send handle "data")
(tcp-recv handle)                 ; => string
(tcp-recv handle 1024)            ; => up to 1024 bytes

;; TCP Server
(tcp-listen 8080)                 ; => server Handle
(tcp-accept server)               ; => client Handle

;; UDP
(udp-socket)                      ; => Handle
(udp-bind handle "0.0.0.0" 5000)
(udp-send-to handle "data" "host" port)
(udp-recv-from handle)            ; => [data host port]

;; Close
(socket-close handle)

;; HTTP Client
(http-get "https://api.example.com/data")
;; => #{:status 200 :body "..." :headers #{...}}

(http-post "https://api.example.com/data"
  :body #{:name "Alice"}
  :headers #{:content-type "application/json"})

(http-request :method :put
              :url "https://api.example.com/item/1"
              :body "data"
              :headers #{:authorization "Bearer token"})

;; HTTP with options
(http-get url
  :timeout 5000
  :follow-redirects true
  :headers #{:accept "application/json"})

;; URL utilities
(url-parse "https://user:pass@host:8080/path?q=1#frag")
;; => #{:scheme "https" :user "user" :password "pass"
;;      :host "host" :port 8080 :path "/path"
;;      :query "q=1" :fragment "frag"}

(url-encode "hello world")        ; => "hello%20world"
(url-decode "hello%20world")      ; => "hello world"

;; Query string
(query-string-parse "a=1&b=2")    ; => #{:a "1" :b "2"}
(query-string-encode #{:a 1 :b 2}) ; => "a=1&b=2"

;; Async operations (with fibers)
(spawn (fn [] (http-get url)))    ; non-blocking
```

### Nick Names Added
```c
OMNI_NAM_SOCK  // Socket handle
OMNI_NAM_TCPC  // TCP connect
OMNI_NAM_TCPL  // TCP listen
OMNI_NAM_TCPA  // TCP accept
OMNI_NAM_TCPS  // TCP send
OMNI_NAM_TCPR  // TCP recv
OMNI_NAM_UDPC  // UDP create
OMNI_NAM_UDPB  // UDP bind
OMNI_NAM_UDPS  // UDP send
OMNI_NAM_UDPR  // UDP recv
OMNI_NAM_SCLS  // Socket close
OMNI_NAM_HTTP  // HTTP request
OMNI_NAM_HGET  // HTTP GET
OMNI_NAM_HPOS  // HTTP POST
OMNI_NAM_HRES  // HTTP response
```

### Functions Implemented
- `@omni_tcp_connect`, `@omni_tcp_listen`, `@omni_tcp_accept`
- `@omni_tcp_send`, `@omni_tcp_recv`
- `@omni_udp_socket`, `@omni_udp_bind`
- `@omni_udp_send_to`, `@omni_udp_recv_from`
- `@omni_socket_close`
- `@omni_http_get`, `@omni_http_post`, `@omni_http_request`
- `@omni_url_parse`, `@omni_url_encode`, `@omni_url_decode`
- `@omni_query_string_parse`, `@omni_query_string_encode`

---

## 12. Developer Tools (`lib/devtools.hvm4`)

**Status:** NOT IN SPEC (mostly)

### Syntax (as implemented)

```lisp
;; Inspect value structure
(inspect value)
;; => #<Int 42>
;; => #<List (1 2 3) length=3>
;; => #<Dict #{:a 1} keys=[:a]>

;; Get runtime type
(type-of 42)              ; => Int
(type-of "hello")         ; => String
(type-of [1 2 3])         ; => Array
(type-of #{:a 1})         ; => Dict

;; Documentation
(doc function-name)       ; => docstring

;; Trace execution
(trace (+ 1 2))
;; Prints: (+ 1 2) => 3
;; => 3

;; Time execution
(time (expensive-operation))
;; Prints: "Elapsed time: 123.45 ms"
;; => result

;; Macro expansion (if macros existed)
(expand '(when true body))
(expand-1 '(when true body))

;; Debug breakpoint (FFI)
(debug)                   ; pauses execution

;; Pretty print
(pprint complex-nested-structure)
;; => formatted output with indentation

;; Get source code
(source function-name)    ; => source string if available

;; Profiling
(profile
  (dotimes [i 1000]
    (expensive-op i)))
;; => #{:total-time 1234 :call-count 1000 :avg-time 1.234}

;; Assertions
(assert (> x 0))
(assert (> x 0) "x must be positive")
```

### Nick Names Added
```c
OMNI_NAM_INSP  // Inspect
OMNI_NAM_TYOF  // Type-of
OMNI_NAM_DOC   // Documentation
OMNI_NAM_TRCE  // Trace
OMNI_NAM_TIME  // Time
OMNI_NAM_EXPD  // Expand
OMNI_NAM_DBUG  // Debug
OMNI_NAM_PRTY  // Pretty-print
OMNI_NAM_SRCE  // Source
OMNI_NAM_PROF  // Profile
```

### Functions Implemented
- `@omni_inspect`
- `@omni_type_of`
- `@omni_doc`
- `@omni_trace`
- `@omni_time`
- `@omni_expand`, `@omni_expand_1`
- `@omni_debug`
- `@omni_pprint`
- `@omni_source`
- `@omni_profile`
- `@omni_assert`

---

## Parser Code Added (Still in parse/_.c)

The parser (`clang/omnilisp/parse/_.c`) still contains parsing code for these features. This code is dormant without runtime implementations:

- Networking: `tcp-connect`, `tcp-listen`, `tcp-accept`, `tcp-send`, `tcp-recv`, `udp-socket`, `udp-bind`, `udp-send-to`, `udp-recv-from`, `socket-close`, `http-get`, `http-post`, `http-request`
- DevTools: `inspect`, `type-of`, `doc`, `trace`, `time`, `expand`, `expand-1`, `debug`, `pprint`, `source`, `profile`, `assert`

## Nick Declarations (Still in nick/omnilisp.c)

The nick file still contains declarations and initializations for:
- Networking nicks (lines 548-563)
- DevTools nicks (lines 577-587)

These can be cleaned up if desired, or left for future use.

---

## Comparison with Original OmniLisp

The user noted that "original OmniLisp has macro system". To clarify:

1. **This project (hvm-omnilisp)** is implementing OmniLisp on HVM4 based on the specification documents in this repo
2. The specification documents (SYNTAX.md, LANGUAGE_REFERENCE.md) do **NOT** define a Lisp-style macro system
3. They only define **reader macros** (`#val`, `#{...}`, `#\char`)

If there's a separate "original OmniLisp" codebase with a macro system, that would need to be reconciled with the spec.

---

## Recommendation

Review each feature and decide:
1. **Add to spec and re-implement** - If the feature is desired
2. **Leave deleted** - If the feature doesn't fit OmniLisp's design
3. **Partial adoption** - Add some functions to spec (e.g., basic string ops might be useful)
