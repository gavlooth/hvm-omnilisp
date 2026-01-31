# HVM4 Parser Compatibility Fixes for runtime.hvm4

## Summary
The runtime.hvm4 file (2743 lines) now parses completely with HVM4.

## Fixes Applied

### 1. Lambda Parameters Need `&` for Multiple Uses
All lambda parameters that are used multiple times need the `&` prefix.

**Before:** `λx. (x + x)`
**After:** `λ&x. (x + x)`

Applied globally via regex: `λname.` → `λ&name.`

### 2. λ{...} Needs Parentheses in !!& Bindings
When a lambda match expression is on the RHS of a `!!&` binding, it must be wrapped in parentheses.

**Before:** `!!&x = λ{0: a; _: b}(y);`
**After:** `!!&x = (λ{0: a; _: b})(y);`

### 3. No Underscore Variables
HVM4 doesn't allow variables starting with `_`. 

**Before:** `λ&_.` or `!!&_ =`
**After:** `λ&ux.` or `!!&uu =`

### 4. No Forward References in Lazy Bindings
Forward references like `!x = f(y); !y = g(x);` don't work.

**Fix:** Use recursive closures (#CloR, #CloK) instead of mutual lazy bindings for named let.

### 5. SUP Syntax Needs Labels
The superposition syntax `&{...}` requires labels.

**Before:** `&{a, b}`
**After:** `&Label{a, b}`

### 6. Function Names Can't Have `?`
HVM4 doesn't allow `?` in identifiers.

**Before:** `@omni_fiber_done?`
**After:** `@omni_fiber_done_p`

### 7. No `:=` Mutation Syntax
HVM4 is pure - no mutable state. Removed mutation operators.

### 8. #Lit Should Return #Cst
For consistency with arithmetic operations, `#Lit{n}` now evaluates to `#Cst{n}` instead of raw `n`.

## Test Suite
Tests are in `test/` directory:
- `test_menv.hvm4` - Meta-environment functions (result: 50)
- `test_env.hvm4` - Environment functions (result: 2)  
- `test_eval_basic.hvm4` - Basic evaluation (result: 156)
- `test_eval_lit.hvm4` - Literal evaluation (result: #Cst{42})

Run all tests: `./test/run_tests.sh`

## Files Modified
- `lib/runtime.hvm4` - Main runtime (all fixes applied)
- `test/*.hvm4` - Test files created
- `test/run_tests.sh` - Test runner script
