# OmniLisp Known Limitations

This document describes known limitations and behaviors in OmniLisp that users should be aware of.

## FFI (Foreign Function Interface) Limitations

### FFI Results Cannot Be Stored in Variables Reliably

FFI results should be used directly in expressions rather than stored in intermediate variables.

**Problem:**
```lisp
;; This may not work correctly
(define d (json-parse "{\"x\": 10}"))
(dict-get d "x")  ;; May return unexpected result

;; This also has issues
(define dt (datetime-now))
(datetime-year dt)  ;; May return Error: 22
```

**Workaround:** Use FFI results directly without intermediate bindings:
```lisp
;; Works correctly
(datetime-year (datetime-now))

;; For JSON, use directly in expressions
(dict-get (json-parse "{\"x\": 10}") "x")
```

**Technical Explanation:** FFI terms (`#FFI{nick, args}`) are not fully dispatched when bound to variables. The dispatch happens at evaluation time, but variable bindings store the unevaluated FFI term. When the variable is later used, the FFI term may not be properly reduced.

### Type Predicates Don't Work With FFI Results

Type checking predicates may not work correctly when the value comes from an FFI call.

**Problem:**
```lisp
;; May return false even for valid arrays
(json-array? (json-parse "[1, 2, 3]"))

;; May return false even for valid objects
(json-object? (json-parse "{\"x\": 1}"))
```

**Workaround:** Parse the JSON first and use the result directly in your logic without checking its type.

---

## Numeric Limitations

### Negative Numbers Wrap Around (u32 Representation)

HVM4 uses unsigned 32-bit integers internally. Negative numbers wrap around to large positive values.

**Problem:**
```lisp
;; Subtraction that would produce negative result
(- 5 10)  ;; Returns 4294967291 (2^32 - 5), not -5

;; Affects datetime-diff when earlier date is first
(datetime-diff earlier-date later-date)  ;; May return large positive number
```

**Workaround:** Always ensure operations produce non-negative results:
```lisp
;; For datetime-diff, put later date first
(datetime-diff later-date earlier-date)

;; For comparisons, use predicates rather than subtraction
(< a b)  ;; Use this instead of checking (- a b) < 0
```

---

## Iterator Limitations

### Range Iterators Are Lazy

Range iterators (`#Rang{...}`) do not automatically convert to lists. They remain as lazy structures.

**Problem:**
```lisp
(range 1 5)  ;; Returns #Rang{1, 5, 1}, not '(1 2 3 4)
```

**Workaround:** Explicitly realize ranges to lists when needed:
```lisp
;; Use take to force realization
(take 10 (range 1 100))  ;; Returns first 10 elements as a list

;; Or use map with identity
(map id (range 1 5))
```

---

## Collection Limitations

### Dict Display May Be Inconsistent

Dicts created through different methods may display differently even if they contain the same data.

**Problem:**
```lisp
#{"a" 1 "b" 2}  ;; Literal syntax
(dict-set (dict-set #{} "a" 1) "b" 2)  ;; Built programmatically

;; These may display differently but are functionally equivalent
```

**Workaround:** Use `dict-get` to access values rather than relying on the printed representation.

---

## Effect System Limitations

### Multi-Shot Continuations Not Fully Implemented

The effect system supports single-shot continuations (each continuation can only be resumed once) but multi-shot continuations (resuming multiple times) have limited support.

**Problem:**
```lisp
(handle
  (perform choose '(1 2 3))
  (choose [options k]
    ;; Resuming multiple times may not work as expected
    (map k options)))
```

**Status:** Partial implementation. Single-shot continuations work reliably.

---

## String Limitations

### String Comparison Returns Numeric Difference

`str-compare` returns the numeric difference between the first differing characters, not a simple -1/0/1 result.

**Example:**
```lisp
(str-compare "apple" "banana")  ;; Returns -1 (a=97, b=98, diff=-1)
(str-compare "cat" "car")       ;; Returns 20 (t=116, r=114, diff=20... wraps to positive)
```

**Workaround:** For ordering comparisons, check if result is 0 (equal) or use string equality:
```lisp
(= (str-compare s1 s2) 0)  ;; True if equal
(str= s1 s2)               ;; Preferred for equality
```

---

## Module System Limitations

### File-Based Module Loading Not Yet Implemented

The `import` statement currently only works with preloaded modules, not file-based loading.

**Current Status:**
```lisp
(import Math)  ;; Works if Math is in prelude
(import "./my-module.omni")  ;; Not yet supported
```

---

## Memory and Performance Considerations

### Parallel Evaluation May Use More Memory

Default parallel functions evaluate more branches than sequential versions, which may increase memory usage.

**Workaround:** Use `_seq` variants for memory-constrained scenarios:
```lisp
(map_seq f xs)     ;; Instead of (map f xs)
(filter_seq p xs)  ;; Instead of (filter p xs)
```

### No Tail Call Optimization for All Patterns

While HVM4 supports efficient recursion, not all recursive patterns are optimized.

**Patterns that work well:**
- Simple structural recursion on lists
- Accumulator-based recursion with strict evaluation

**Patterns to avoid:**
- Deep mutual recursion without clear structure
- Unbounded recursion without base cases

---

## Debugging Limitations

### Limited Stack Traces

Error messages may not include full stack traces, making debugging difficult.

**Workaround:** Add explicit debug prints at key points:
```lisp
(println "Debug: value is" value)
```

### FFI Errors Return Numeric Codes

FFI errors return errno codes rather than descriptive messages.

**Common codes:**
- `Error: 22` = EINVAL (Invalid argument)
- `Error: 2` = ENOENT (No such file or directory)
- `Error: 13` = EACCES (Permission denied)

---

## Summary Table

| Limitation | Category | Severity | Workaround Available |
|------------|----------|----------|---------------------|
| FFI results in variables | FFI | High | Yes - use directly |
| Type predicates + FFI | FFI | Medium | Yes - avoid checking |
| Negative number wrap | Numeric | Medium | Yes - ensure positive |
| Lazy ranges | Iterator | Low | Yes - explicit realize |
| Multi-shot continuations | Effects | Medium | Partial |
| String compare result | String | Low | Yes - check for 0 |
| File-based modules | Module | High | No - not implemented |
| Parallel memory usage | Performance | Low | Yes - use _seq |

---

## Reporting Issues

If you encounter a limitation not documented here, please report it with:
1. Minimal reproduction code
2. Expected behavior
3. Actual behavior
4. OmniLisp version/commit hash
