# OmniLisp Runtime Hardening Strategy

This document outlines a comprehensive strategy for hardening `lib/runtime.hvm4` and ensuring it works as expected.

## Runtime Architecture Overview

The runtime.hvm4 file (2743 lines) implements the OmniLisp interpreter in HVM4. Key components:

| Component | Lines | Functions | Description |
|-----------|-------|-----------|-------------|
| Meta-Environment | 1-50 | `@omni_menv_*` | Multi-level environments for tower |
| Environment Ops | 50-80 | `@omni_env_*` | De Bruijn indexed bindings |
| Core Evaluator | 80-410 | `@omni_eval` | Main expression evaluator |
| Function Application | 413-500 | `@omni_apply` | Closure/generic function dispatch |
| Generic Dispatch | 472-670 | `@omni_find_best_method` | Multi-dispatch with ambiguity detection |
| List Evaluation | 675-700 | `@omni_eval_list*` | List processing helpers |
| Arithmetic | 705-805 | `@omni_add/sub/mul/div/mod` | Numeric operations |
| Pattern Matching | 810-930 | `@omni_match`, `@omni_pattern_match` | Case analysis |
| CPS Evaluator | 930-1110 | `@omni_eval_cps` | Delimited continuations |
| Effects | 1115-1220 | `@omni_handle_cps`, `@omni_perform_cps` | Algebraic effects |
| Proof System | 1222-1460 | `@omni_proof_handler_*` | Contract verification |
| Fibers | 1462-1600 | `@omni_fiber_*` | Cooperative concurrency |
| Tower | 1600-1700 | `@omni_reflect`, `@omni_reify` | Multi-stage programming |
| Type System | 1700-1740 | `@omni_type_of`, `@type_*` | Runtime type descriptors |
| Helpers | 1743-2743 | Various | Lists, pipes, paths, etc. |

## Hardening Strategy

### 1. Unit Test Coverage

Create HVM4-native tests for each runtime function in `test/runtime/`:

```
test/runtime/
├── test_menv.hvm4         # Meta-environment operations
├── test_env.hvm4          # Environment operations
├── test_eval_basic.hvm4   # Core evaluator - basic cases
├── test_eval_control.hvm4 # Control flow (if/cond/when)
├── test_apply.hvm4        # Function application
├── test_dispatch.hvm4     # Generic function dispatch
├── test_arithmetic.hvm4   # Arithmetic operations
├── test_pattern.hvm4      # Pattern matching
├── test_cps.hvm4          # CPS evaluator
├── test_effects.hvm4      # Algebraic effects
├── test_fibers.hvm4       # Fiber concurrency
├── test_tower.hvm4        # Reflect/reify
├── test_types.hvm4        # Type checking
├── test_list.hvm4         # List operations
├── test_path.hvm4         # Path access (get/assoc)
└── test_pipe.hvm4         # Pipe operator
```

### 2. Test Categories

#### 2.1 Correctness Tests
- Each function produces expected output for known inputs
- Edge cases (empty lists, zero, negative numbers)
- Error conditions return proper `#Err{}` values

#### 2.2 Property-Based Tests
- Associativity: `(+ (+ a b) c) == (+ a (+ b c))`
- Identity: `(+ x 0) == x`
- Inverse operations: Pattern match → reconstruct equals original
- Type preservation: Operations maintain type invariants

#### 2.3 Stress Tests
- Deep recursion (stack safety)
- Large data structures
- Many concurrent fibers

#### 2.4 Error Handling Tests
- Division by zero → `#Err{#sym_divz}`
- Unbound variable → `#Err{#sym_unbound}`
- Type mismatch → `#Err{#sym_type}`
- No handler → `#Err{#sym_nohandler, tag}`
- No match → `#Err{#sym_nomatch}`

### 3. Critical Functions to Test

#### High Priority (Core Execution)
| Function | Tests Required |
|----------|---------------|
| `@omni_eval` | All AST node types |
| `@omni_apply` | Closures, recursive closures, generics, partials |
| `@omni_eval_cps` | All CPS-mode nodes |
| `@omni_apply_cps` | CPS closures, continuations |
| `@omni_perform_cps` | Effect invocation |
| `@omni_match` | All pattern types |

#### Medium Priority (Data Operations)
| Function | Tests Required |
|----------|---------------|
| `@omni_add/sub/mul/div/mod` | Basic ops, edge cases, errors |
| `@omni_eql/lt/gt` | Comparisons, type handling |
| `@omni_get/get_in` | List, dict, nested access |
| `@omni_assoc/assoc_in` | Functional updates |
| `@omni_pattern_match` | All pattern variants |

#### Lower Priority (Support Functions)
| Function | Tests Required |
|----------|---------------|
| `@omni_list_*` | Length, append, reverse |
| `@omni_reflect/reify` | Round-trip preservation |
| `@omni_fiber_*` | Spawn, yield, resume |

### 4. Test Infrastructure

#### HVM4 Test Harness
```hvm4
// test/runtime/harness.hvm4
@assert_eq = λ&name. λ&expected. λ&actual.
  λ{
    1: #Pass{name}
    _: #Fail{name, expected, actual}
  }((expected == actual))

@run_tests = λ&tests.
  λ{
    #NIL: #Done{}
    #CON: λ&test. λ&rest.
      !!&result = test;
      λ{
        #Pass: λ&n. @run_tests(rest)
        #Fail: λ&n. λ&e. λ&a. #Fail{n, e, a}
      }(result)
  }(tests)
```

#### Integration with Existing Test Runner
The HVM4 tests can be run via `test/run_tests.sh` which already handles `.hvm4` files.

### 5. Specific Invariants to Verify

#### 5.1 Environment Invariants
- `@omni_env_extend(env)(v)` then `@omni_env_get(env')(0)` returns `v`
- Index beyond bounds → `#Err{#sym_unbound}`

#### 5.2 Arithmetic Invariants
- `@omni_add(#Cst{a})(#Cst{b})` == `#Cst{(a + b)}`
- `@omni_div(x)(#Cst{0})` == `#Err{#sym_divz}`

#### 5.3 Pattern Match Invariants
- Wildcard `#PWld` matches anything, produces `#NIL` bindings
- Variable `#PVar{i}` captures scrutinee
- Constructor `#PCtr{tag, args}` matches only same tag

#### 5.4 CPS Invariants
- `@omni_eval_cps(m)(e)(λ&v.v)` behaves like `@omni_eval(m)(e)` for non-control expressions
- `#Ctrl` captures the continuation
- Nested `#Prmt` creates independent delimiters

#### 5.5 Effect Handler Invariants
- Handler lookup finds most recently installed handler
- Resume invokes captured continuation
- Missing handler → `#Err{#sym_nohandler, tag}`

### 6. Error Categorization

| Error Type | Symbol | Condition |
|------------|--------|-----------|
| Unbound variable | `#sym_unbound` | De Bruijn index out of bounds |
| Type error | `#sym_type` | Operation on wrong type |
| Division by zero | `#sym_divz` | Divide/mod by zero |
| No match | `#sym_nomatch` | Pattern match exhausted |
| No handler | `#sym_nohandler` | Effect with no handler |
| No method | `#sym_NoMethod` | Generic dispatch failed |
| Ambiguous | `#sym_AmbiguousMethod` | Multiple equally-specific methods |
| CPS error | `#sym_cps` | CPS mode invariant violated |
| Application error | `#sym_app` | Apply to non-function |

### 7. Regression Test Suite

For each bug found:
1. Create minimal reproduction test
2. Add to `test/runtime/test_regressions.hvm4`
3. Document the bug and fix

### 8. Performance Benchmarks

Track performance of critical paths:
- `@omni_eval` dispatch overhead
- `@omni_apply` function call cost
- `@omni_match` pattern matching
- `@omni_find_best_method` dispatch resolution

### 9. Implementation Checklist

- [ ] Create `test/runtime/` directory
- [ ] Implement test harness in HVM4
- [ ] Write tests for `@omni_env_*` functions
- [ ] Write tests for `@omni_menv_*` functions
- [ ] Write tests for arithmetic operations
- [ ] Write tests for comparisons
- [ ] Write tests for pattern matching
- [ ] Write tests for `@omni_eval` basic cases
- [ ] Write tests for `@omni_eval` control flow
- [ ] Write tests for CPS evaluator
- [ ] Write tests for effects
- [ ] Write tests for fibers
- [ ] Write tests for tower operations
- [ ] Write tests for type checking
- [ ] Write tests for list/dict operations
- [ ] Set up CI integration
- [ ] Document all test coverage

## Running the Tests

```bash
# Run all runtime tests
./test/run_tests.sh

# Run specific component
./hvm4/clang/main test/runtime/test_arithmetic.hvm4
```

## Maintenance

- Review runtime changes against tests
- Add tests for new features
- Update invariants when semantics change
