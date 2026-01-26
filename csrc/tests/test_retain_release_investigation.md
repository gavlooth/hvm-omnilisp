# test-retain-release-generation Investigation Summary

## Status

The test file `test_region_release_generation.c` already exists and covers testing of retain/release generation APIs.

## Existing Test Coverage

The file `csrc/tests/test_region_release_generation.c` contains 3 tests:

1. **test_region_release_function_exists** - Verifies `omni_codegen_emit_region_releases_at_pos()` can be called without crashing
2. **test_escape_repair_emits_retain** - Verifies that calling `omni_codegen_escape_repair()` with `ESCAPE_REPAIR_RETAIN_REGION` strategy emits `region_retain_internal(_local_region)`
3. **test_escape_repair_emits_transmigrate** - Verifies that calling `omni_codegen_escape_repair()` with `ESCAPE_REPAIR_TRANSMIGRATE` strategy emits `transmigrate(test_var, _local_region, _caller_region)` and does NOT emit `region_retain_internal`

All 3 tests pass.

## Current Implementation Status

1. **Low-level API** (`omni_codegen_escape_repair()`, `omni_codegen_emit_region_releases_at_pos()`) - **IMPLEMENTED**
   - Both RETAIN_REGION and TRANSMIGRATE strategies work correctly
   - Tests verify this

2. **Full Code Generation** (`omni_codegen_program()`) - **PARTIALLY IMPLEMENTED**
   - `codegen_define()` (line 1869 in codegen.c) hardcodes `ESCAPE_REPAIR_TRANSMIGRATE`
   - No support for selecting RETAIN_REGION strategy at compile time
   - No environment variable or configuration option to select strategy

3. **Region-RC Last-Use Release Generation** - **IMPLEMENTED BUT NOT INTEGRATED**
   - `omni_codegen_emit_region_releases_at_pos()` is implemented in region_codegen.c
   - Function is called at positions in codegen.c (lines 3199, 3272, 3279)
   - However, tests show that `region_release_internal(_local_region)` is not appearing in generated output for test programs
   - This suggests either:
     a) The test programs don't trigger last-use scenarios correctly
     b) The release generation logic has bugs
     c) The variable names don't match what tests expect

## What TODO is Asking For

From TODO.md Issue 1 P2:

> Objective: Extend CTRR so that when it chooses "retain region" (instead of transmigrate), it also emits matching `region_retain_internal()` and `region_release_internal()` at compile time based on last-use, so regions can outlive scope safely without leaks.

This requires:
1. [ ] Allow selection of RETAIN_REGION strategy (vs hardcoding TRANSMIGRATE)
2. [ ] Integrate `region_release_internal()` emission into full codegen flow
3. [ ] Ensure last-use analysis correctly identifies when to emit release_internal

## Recommended Next Steps

To complete this TODO item:

1. Add strategy selection mechanism (environment variable, compiler flag, or analysis-based selection)
2. Modify `codegen_define()` to use selected strategy instead of hardcoding TRANSMIGRATE
3. Debug why `region_release_internal()` doesn't appear in generated output:
   - Check if last-use analysis is correctly identifying variables
   - Check if `omni_codegen_emit_region_releases_at_pos()` is being called correctly
   - Verify variable name matching between codegen and tests
4. Add end-to-end integration test that compiles OmniLisp program and verifies retain/release in generated C
