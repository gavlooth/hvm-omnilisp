# Test Coverage Summary: prim_leak_check

## Date
2025-01-15

## Test File
`runtime/tests/test_leak_check.c`

## Function Tested
`prim_leak_check` - A debugging primitive that returns a dictionary with allocation statistics

## Location
`runtime/src/debug.c` (line ~557)

## Purpose
The `prim_leak_check` function is a developer tool for debugging memory issues in OmniLisp programs. It:
- Tracks total allocations and frees
- Estimates potential memory leaks
- Returns diagnostic information as a dictionary

## Test Coverage

### Tests Added (6 tests)

1. **leak_check_returns_dict**
   - Verifies that the function returns a valid dictionary object
   - Tests return type and null-check

2. **leak_check_has_expected_keys**
   - Verifies that the returned dictionary contains expected keys:
     - `:estimated-leaks`
     - `:total-allocs`
     - `:total-frees`

3. **leak_check_values_non_negative**
   - Ensures all returned values are >= 0
   - Tests data validity

4. **leak_check_is_idempotent**
   - Tests that calling the function multiple times doesn't crash
   - Verifies consistent return structure across calls

5. **leak_check_after_allocations**
   - Tests the function after creating objects
   - Verifies that allocations are tracked

6. **leak_check_estimates_correctly**
   - Verifies the formula: `estimated = max(0, allocs - frees)`
   - Tests calculation accuracy

## API Coverage

### Input Parameters
- None (void function)

### Return Value
- Dictionary with the following keys:
  - `:estimated-leaks` - Integer count of estimated leaked objects
  - `:total-allocs` - Integer count of total allocations
  - `:total-frees` - Integer count of total frees

## Testing Strategy

### 1. Type Safety
- Verify return type is `TAG_DICT`
- Ensure no NULL returns

### 2. Data Integrity
- Check that all values are non-negative
- Verify mathematical relationship between values

### 3. Functional Correctness
- Test idempotency (multiple calls)
- Verify tracking across allocations

### 4. Edge Cases
- Empty allocation state (no allocations)
- Repeated calls without changes

## Implementation Notes

The test file includes:
- Custom test framework macros (TEST, RUN_TEST, ASSERT)
- Helper function `dict_get_int` for extracting dictionary values
- Minimal dependencies on OmniLisp runtime

## Compilation

```bash
cd runtime/tests
gcc -I../include -I../../csrc test_leak_check.c -o test_leak_check -L.. -lomni
```

## Execution

```bash
./test_leak_check
```

Expected output:
```
=== prim_leak_check Tests ===

--- Core Functionality ---
  Testing leak_check_returns_dict... PASSED
  Testing leak_check_has_expected_keys... PASSED
  Testing leak_check_values_non_negative... PASSED
  Testing leak_check_is_idempotent... PASSED

--- Data Accuracy ---
  Testing leak_check_estimates_correctly... PASSED
  Testing leak_check_after_allocations... PASSED

=== Test Results ===
Total: 6
Passed: 6
Failed: 0
ALL TESTS PASSED!
```

## Related Documentation
- `runtime/docs/README.md` - Runtime developer guide
- `runtime/src/debug.c` - Implementation file

## Impact
This test adds coverage to a critical debugging primitive that helps developers:
- Identify memory leaks in programs
- Track allocation patterns
- Profile memory usage

The test ensures that the primitive:
- Returns valid data structures
- Calculates estimates correctly
- Doesn't crash on repeated calls
- Works across various allocation scenarios
