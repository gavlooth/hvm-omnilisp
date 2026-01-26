# Gap Analysis: hvm-omnilisp vs Original OmniLisp

This document compares features in the hvm-omnilisp implementation against the original OmniLisp at `~/code/OmniLisp`.

## Legend
- ✅ Implemented
- ⚠️ Partial (different syntax or incomplete)
- ❌ Missing
- 🔄 HVM4 provides (via runtime, not parser)

---

## 1. Core Language Features

| Feature | Status | Notes |
|---------|--------|-------|
| `define` | ✅ | Function/value/type definitions |
| `let` bindings | ✅ | Local bindings |
| `lambda`/`fn` | ✅ | Anonymous functions |
| `if`/`when`/`unless` | ✅ | Conditionals |
| `match` | ✅ | Pattern matching (replaces cond/case) |
| `quote` | ✅ | Quote expressions |
| `begin`/`do` | ✅ | Sequence expressions |
| `module`/`import` | ✅ | Module system |

**Note**: `cond`/`case` are intentionally NOT supported - use `match` for branching. Clojure-style `loop`/`recur` are NOT supported - use **named let** (Scheme-style) for iteration:
```lisp
(let loop [i 0] [sum 0]
  (if (< i 10)
      (loop (+ i 1) (+ sum i))
      sum))
```

---

## 2. Function Combinators

| Feature | Status | Notes |
|---------|--------|-------|
| `\|>` pipe | ✅ | PREFIX: `(\|> 5 inc square)` |
| `curry` | ✅ | With optional arity: `(curry f 2)` |
| `flip` | ✅ | Swap first two arguments |
| `rotate` | ✅ | Cycle arguments left |
| `comp` | ✅ | Function composition |
| `apply` | ✅ | Apply function to arg list |
| `partial` | ❌ | Removed in favor of `curry` |
| `->` arrow lambda | ❌ | Shorthand: `(-> x (* x x))` |
| `identity` | ❌ | Identity function |
| `constantly` | ❌ | Always return same value |

---

## 3. Pattern Matching

| Feature | Status | Notes |
|---------|--------|-------|
| Constructor patterns | ✅ | `[Point(x y) ...]` |
| Literal patterns | ✅ | `[0] [1] ["str"]` |
| Wildcard `_` | ✅ | Match anything |
| Variable binding | ✅ | `[x]` binds to x |
| Rest/spread `..` | ✅ | `[h .. t]` in list patterns |
| Guard `&` | ✅ | `[x & (> x 0)]` |
| `:when` guard | ✅ | `[x :when (> x 0)]` in match clause |
| `as` patterns | ✅ | `[Point(x y) as p]` |
| String patterns | ✅ | `["hello"]` matches string |
| Or patterns | ❌ | `[0 \| 1]` match either |
| View patterns | ❌ | `[(@ len n) & (> n 0)]` |

---

## 4. Types and Definitions

**OmniLisp does NOT use `deftype`/`defstruct`/`defenum`.** All definitions use `define` with placeholders:

```lisp
;; Type definition via define with slot placeholder
(define Point [x {Int}] [y {Int}])

;; Creates constructor Point and accessors Point-x, Point-y
(Point 3 4)        ; construct
(Point-x p)        ; access x field

;; Enum/variant types
(define Color
  Red
  Green
  Blue)

;; Parametric types
(define Maybe [T]
  Nothing
  (Just [val {T}]))
```

| Feature | Status | Notes |
|---------|--------|-------|
| Type annotations `{Type}` | ✅ | In slots: `[x {Int}]` |
| Return type | ✅ | `(define f [x {Int}] {Int} ...)` |
| ADT via `define` | ✅ | `(define Maybe Nothing (Just val))` |
| Gradual multiple dispatch | ✅ | Via `define` with type patterns, compile-time when typed |
| Type parameters | ✅ | `(define Maybe [T] ...)` |

---

## 5. Mutation Primitives

| Feature | Status | Notes |
|---------|--------|-------|
| `set!` | ✅ | Mutate variable: `(set! x 10)` |
| `put!` | ✅ | Mutate dict in place: `(put! d :k v)` |
| `update!` | ✅ | Update via fn: `(update! d :k inc)` |
| `swap!` | ❌ | Atomic swap (for concurrency) |
| `reset!` | ❌ | Reset atom value |

**Note**: Use `^:mutable` metadata to mark bindings as mutable.

---

## 6. Effects System

| Feature | Status | Notes |
|---------|--------|-------|
| `perform` | ✅ | Perform an effect |
| `handle` | ✅ | Effect handler |
| `reset` | ✅ | Delimited continuation reset |
| `shift` | ✅ | Delimited continuation shift |
| `control` | ✅ | One-shot continuation |
| `yield` | ✅ | Generator yield |

**Note**: CL-style condition system (handler-case, restart-case) can be built on top of effects.

---

## 7. Collections - Lists

| Feature | Status | Notes |
|---------|--------|-------|
| `list` | ✅ | Create list |
| `cons` | ✅ | Prepend element |
| `first`/`car` | ✅ | First element |
| `rest`/`cdr` | ✅ | Rest of list |
| `nth` | ✅ | Get nth element |
| `reverse` | ❌ | Reverse list |
| `concat` | ❌ | Concatenate lists |
| `flatten` | ❌ | Flatten nested lists |
| `length`/`count` | ❌ | Collection size |
| `empty?` | ❌ | Check if empty |
| `member?`/`contains?` | ❌ | Membership test |

---

## 8. Iterators (Lazy, Pull-Based)

OmniLisp uses **iterator-based lazy evaluation**, not Clojure-style lazy sequences.

| Feature | Status | Notes |
|---------|--------|-------|
| `range` | ✅ | `(range 10)`, `(range 0 10 2)` |
| `iter-map` | ✅ | Lazy map |
| `iter-filter` | ✅ | Lazy filter |
| `take` | ✅ | Take n elements |
| `drop` | ✅ | Drop n elements |
| `take-while` | ✅ | Take while predicate true |
| `drop-while` | ✅ | Drop while predicate true |
| `zip` | ✅ | Zip multiple iterators |
| `chain` | ✅ | Concatenate iterators |
| `enumerate` | ✅ | Add indices |
| `fold`/`reduce` | ✅ | Fold/reduce |
| `find` | ✅ | Find first matching |
| `any?` | ✅ | Any matches? |
| `all?` | ✅ | All match? |
| `nth` | ✅ | Get nth element |
| `flat-map` | ✅ | Map then flatten |
| `step-by` | ✅ | Take every nth |
| `chunks` | ✅ | Group into chunks |
| `windows` | ✅ | Sliding windows |
| `list` | ✅ | Collect to list (dispatch) |
| `array` | ✅ | Collect to array (dispatch) |
| `iterate` | ✅ | Infinite lazy sequence |
| `repeat` | ✅ | Repeat value |
| `cycle` | ✅ | Cycle through collection |

---

## 9. Collections - Dict

| Feature | Status | Notes |
|---------|--------|-------|
| `get` | ✅ | Get value (generic dispatch) |
| `put` | ✅ | Set value, functional (generic dispatch) |
| `update` | ✅ | Update value (generic dispatch) |
| `get-in` | ✅ | Nested get |
| `assoc-in` | ✅ | Nested set |
| `update-in` | ✅ | Nested update |
| `remove` | ✅ | Remove key (generic dispatch) |
| Dict literal `#{...}` | ✅ | `#{"a" 1 "b" 2}` |
| `keys` | ✅ | Get all keys (generic dispatch) |
| `values` | ✅ | Get all values (generic dispatch) |
| `entries` | ✅ | Get key-value pairs |
| `merge` | ✅ | Merge dicts (generic dispatch) |
| `contains?` | ✅ | Check key exists (generic dispatch) |
| `select-keys` | ❌ | Select subset of keys |
| Dict destructuring | ❌ | `(let [{a :a} dict] ...)` |

---

## 10. Strings

| Feature | Status | Notes |
|---------|--------|-------|
| String literals | ✅ | `"hello"` |
| String ops (nicks exist) | ⚠️ | Nicks defined, need parser |

String operations have nicks defined but need parser integration.

---

## 11. Regex (Pika-Based)

| Feature | Status | Notes |
|---------|--------|-------|
| Regex literal | ✅ | `#r"pattern"flags` |
| `re-match` | ✅ | Match entire string |
| `re-find` | ✅ | Find first match |
| `re-find-all` | ✅ | Find all matches |
| `re-replace` | ✅ | Replace matches |
| `re-split` | ✅ | Split by pattern |
| `re-groups` | ✅ | Get capture groups |

**Note**: Regex is implemented using the Pika parsing algorithm, not PCRE.

---

## 12. I/O and File System

| Feature | Status | Notes |
|---------|--------|-------|
| `read-file`/`slurp` | ✅ | Read file contents |
| `write-file`/`spit` | ✅ | Write file |
| `append-file` | ✅ | Append to file |
| `read-lines` | ✅ | Read lines |
| `print`/`println` | ✅ | Console output |
| `read-line` | ✅ | Console input |
| `file-exists?` | ✅ | Check existence |
| `dir?` | ✅ | Is directory? |
| `mkdir` | ✅ | Create directory |
| `list-dir` | ✅ | List directory |
| `delete-file` | ✅ | Delete file |
| `rename-file` | ✅ | Rename/move |
| `copy-file` | ✅ | Copy file |
| `getenv`/`setenv` | ✅ | Environment variables |

---

## 13. Network

| Feature | Status | Notes |
|---------|--------|-------|
| `tcp-connect` | ✅ | TCP client |
| `tcp-listen` | ✅ | TCP server |
| `tcp-accept` | ✅ | Accept connection |
| `tcp-send`/`tcp-recv` | ✅ | Send/receive |
| `udp-socket` | ✅ | UDP socket |
| `udp-send-to`/`udp-recv-from` | ✅ | UDP send/recv |
| `socket-close` | ✅ | Close socket |
| `http-get`/`http-post` | ✅ | HTTP client |
| `http-request` | ✅ | Generic HTTP |

---

## 14. JSON

| Feature | Status | Notes |
|---------|--------|-------|
| `json-parse` | ✅ | Parse JSON |
| `json-stringify` | ✅ | Encode JSON |
| `json-get` | ✅ | Get JSON field |
| `json-get-in` | ✅ | Nested get |

---

## 15. Date/Time

| Feature | Status | Notes |
|---------|--------|-------|
| `datetime-now` | ✅ | Current time |
| `datetime-parse` | ✅ | Parse datetime |
| `datetime-format` | ✅ | Format datetime |
| `datetime-add`/`sub`/`diff` | ✅ | Arithmetic |
| Component getters | ✅ | year, month, day, etc. |
| `duration` | ✅ | Create duration |
| `days`/`hours`/`minutes`/`seconds` | ✅ | Duration helpers |

---

## 16. Math

| Feature | Status | Notes |
|---------|--------|-------|
| Basic: `+`, `-`, `*`, `/`, `mod` | ✅ | Arithmetic |
| Comparison: `=`, `<`, `>`, `<=`, `>=` | ✅ | |
| `sqrt`, `pow`, `exp`, `log` | ✅ | |
| Trig: `sin`, `cos`, `tan`, etc. | ✅ | |
| `abs`, `floor`, `ceil`, `round` | ✅ | |
| `min`, `max` | ✅ | |
| `random` | ✅ | |
| `inc`/`dec` | ❌ | Increment/decrement |
| `even?`/`odd?` | ❌ | Predicates |
| Bitwise ops | ❌ | bit-and, bit-or, etc. |

---

## 17. Meta-Programming

| Feature | Status | Notes |
|---------|--------|-------|
| `EM`/`eval-meta` | ✅ | Meta-level evaluation |
| `clambda`/`staged-fn` | ✅ | Compiled lambda |
| `stage` | ✅ | Stage expression |
| `splice` | ✅ | Splice into code |
| `reflect`/`reify` | ✅ | Reflection |
| `meta-level` | ✅ | Get meta level |
| `with-meta-env` | ✅ | Scoped meta env |
| `lift`/`run` | ✅ | Lift/run computations |
| `quote` | ✅ | Quote expressions |

---

## 18. Debugging

| Feature | Status | Notes |
|---------|--------|-------|
| `inspect` | ✅ | Inspect value |
| `type-of` | ✅ | Get type |
| `doc` | ✅ | Get documentation |
| `trace` | ✅ | Trace execution |
| `time` | ✅ | Time execution |
| `expand`/`expand-1` | ✅ | Macro expansion |
| `debug` | ✅ | Debug mode |
| `pprint` | ✅ | Pretty print |
| `source` | ✅ | Get source |
| `profile` | ✅ | Profile execution |
| `assert` | ✅ | Assert condition |

---

## 19. FFI

| Feature | Status | Notes |
|---------|--------|-------|
| `ffi` call | ✅ | Call C function |
| Handle-based safety | 🔄 | In runtime design |
| Ownership annotations | 🔄 | `^:owned`, `^:borrowed` |
| Type marshaling | ❌ | Auto type conversion |

---

## 20. Grammar DSL (Pika Parser)

| Feature | Status | Notes |
|---------|--------|-------|
| `grammar` definition | ⚠️ | Nicks defined, need full DSL |
| `rule` definitions | ⚠️ | Nicks defined |
| Parser combinators | ⚠️ | seq, alt, rep defined |
| Parse actions | ⚠️ | Nicks defined |

---

## Key Design Decisions

### No `deftype`/`defstruct`/`defenum`
OmniLisp uses `define` for everything:
```lisp
;; This is how you define types
(define Point [x {Int}] [y {Int}])      ;; struct-like
(define Color Red Green Blue)            ;; enum-like
(define Maybe [T] Nothing (Just [v {T}])) ;; ADT with type param
```

### No `cond`/`case` - No Clojure `loop`/`recur`
- Use `match` for multi-way branching
- Use **named let** (Scheme-style) for iteration:
  ```lisp
  (let loop [i 0] [acc '()]
    (if (< i 10)
        (loop (+ i 1) (cons i acc))
        acc))
  ```
- Use iterators for lazy sequences

### Iterator-Based Lazy (Not Clojure-Style)
- Pull-based iterators instead of lazy cons cells
- Avoids complex persistent data structures
- Better for HVM4's optimal reduction

### Pika-Based Regex
- Regex patterns compiled to Pika grammar
- Same parser technology throughout
- Captures as grammar captures
