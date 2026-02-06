# Pattern Matching Status

## Working ✅
- Basic literal patterns (`0`, `"str"`, etc.)
- Wildcard patterns (`_`)
- Array patterns (`[x y]`)
- List patterns (`(h .. t)`)
- Case/match statements
- Guard patterns
- Or patterns (basic)

## Needs Work ⚠️
- **As-patterns**: `[x y] as pair` syntax not parsing correctly
  - Parser doesn't handle `as` keyword in pattern context
  - Error: "expected: ), detected: EOF"
  - Files: test_pattern_as.omni
  
- **Complex nested patterns**: Deep nesting with multiple `.` operators
  - Error on patterns like `(((a .. _) .. _) .. _)`
  - Files: test_pattern_nested.omni

- **Or-patterns with quotes**: `(| 'apple 'banana)` not parsing
  - Quote inside or-pattern context causes issues
  - May be fixed now, needs verification

## Tests
- Passing: test_pattern_literals.omni, test_pattern_wildcard.omni, test_pattern_array.omni, test_pattern_binding.omni, test_case.omni, test_list_patterns.omni, test_guard_pattern.omni
- Failing: test_pattern_as.omni, test_pattern_nested.omni

## Next Steps
1. Add `as` keyword handling in pattern parser
2. Debug nested pattern parsing for complex rest patterns
3. Verify or-pattern handling with quoted literals
