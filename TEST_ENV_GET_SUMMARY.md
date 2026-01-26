# Test Coverage Added - env-get Primitive

## Summary
Added comprehensive test coverage for `env-get` primitive function from `runtime/src/debug.c`.

## Files Created
- `tests/test_env_get.lisp` - Comprehensive test suite (22 test cases)

## Files Modified
- `runtime/src/debug.c` - Added `// TESTED - tests/test_env_get.lisp` marker at line 809

## Function Tested
**`prim_env_get`** - Look up a symbol in the current environment

### Function Signature
```c
Obj* prim_env_get(Obj* sym)
```

### Description
The `env-get` primitive retrieves the value bound to a symbol in the current lexical environment. It accepts either a symbol or string as input and returns the bound value, or `NULL` if the symbol is not found.

## Test Coverage

### Test Categories (22 total tests)

#### 1. Basic env-get Tests (3 tests)
- Get defined integer variable
- Get string variable  
- Get boolean variable

#### 2. Multiple Variables (3 tests)
- Multiple independent variables (a, b, c)
- Verify each variable is accessible

#### 3. Variable Shadowing (1 test)
- Inner variable shadows outer in scope
- Outer variable accessible after block

#### 4. Nil/Nothing Handling (2 tests)
- Get `nothing` variable
- Get zero value

#### 5. Complex Types (4 tests)
- Get list variable
- Get array variable
- Get dict variable
- Get function/closure variable

#### 6. Variable Rebinding (1 test)
- Rebind variable and verify new value

#### 7. Floating Point Values (2 tests)
- Get float variable (π)
- Get negative float

#### 8. Special Values (2 tests)
- Get negative integer
- Get large positive integer

#### 9. Variable Name Patterns (4 tests)
- Hyphenated variable name (`my-var`)
- Numbered variable name (`var2`)
- Question-marked name (`done?`)
- Exclamation-marked name (`important!`)

#### 10. Nested Structures (2 tests)
- Get nested list
- Get array of arrays (matrix)

## Test Format

The test follows the project's standard test conventions:

```lisp
(define test-count 0)
(define pass-count 0)
(define fail-count 0)

(define test-eq [name] [expected] [actual]
  (set! test-count (+ test-count 1))
  (if (= expected actual)
      (do (set! pass-count (+ pass-count 1))
           (print "PASS:" name))
      (do (set! fail-count (+ fail-count 1))
           (print "FAIL:" name)
           (print "  Expected:" expected)
           (print "  Got:" actual))))

;; Test cases here...

(print "")
(print "=== Test Results ===")
(print "Total:" test-count)
(print "Passed:" pass-count)
(print "Failed:" fail-count)

(if (= fail-count 0)
    (print "ALL TESTS PASSED!")
    (print "SOME TESTS FAILED"))

fail-count  ;; Return 0 for success
```

## Execution

Run the test with:
```bash
./csrc/omnilisp tests/test_env_get.lisp
```

## Source Code Marker

Added the following marker to `runtime/src/debug.c` at line 809:
```c
// TESTED - tests/test_env_get.lisp
```

This marker indicates that the function has comprehensive test coverage.

## Related Functions

Other untested debug functions (for future coverage):
- `prim_doc` - Get documentation for symbol/function
- `prim_source` - Get source code for function
- `prim_env_bindings` - Get all environment bindings
- `prim_region_stats` - Print region allocation statistics
- `prim_memory_usage` - Return total memory allocated
- `prim_allocation_trace` - Toggle allocation tracing
- `prim_leak_check` - Scan for potential memory leaks

## Notes

- The test covers both symbols and strings as lookup keys (as `prim_env_get` supports both via TAG_SYM and TAG_STRING tags)
- Edge cases include nil/nothing values, zero, negative numbers, and large integers
- Variable naming conventions tested include common Lisp patterns: hyphens, numbers, and predicate suffixes (?)
