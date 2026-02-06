# Macro Syntax Fixes Required

## Problem

Documentation and test files use **Scheme-style** macro syntax:
- `define-syntax` (Scheme convention)
- `syntax-rules` (Scheme pattern system)
- Two separate macro systems

But **OmniLisp actually uses**:
- `define [syntax name]` (universal define form)
- Pattern matching with `[(pattern) template]`
- **ONE** pattern-based macro system

## Affected Files

### Design Documents
1. `clang/PHASE_4_2_DESIGN.md` - 37 occurrences
2. `docs/IMPLEMENTATION_COMPARISON.md` - 1 occurrence

### Test Files
- `test_syntax_hygiene.omni`
- `test_syntax_ellipsis.omni`
- `test_macroexpand_all.omni`
- `test_syntax_pattern.omni`
- `test_syntax_literals.omni`
- `test_define_syntax.omni`

## Correct OmniLisp Syntax

### Before (Scheme-style - WRONG)
```scheme
(define-syntax when
  (syntax-rules ()
    [(_ test body ...)
     (if test (do body ...) unit)]))
```

### After (OmniLisp - CORRECT)
```lisp
(define [syntax when]
  [(when ?test ?body ...)
   (if ?test (do ?body ...) unit)])
```

## Key Differences

| Scheme | OmniLisp |
|--------|----------|
| `define-syntax name` | `define [syntax name]` |
| `(syntax-rules (literals) ...)` | Direct pattern rules |
| `_` wildcard | Named parameters |
| No unquote in templates | Use `?var` for captures |

## Action Plan

1. ✅ Document the issue (this file)
2. [ ] Rewrite PHASE_4_2_DESIGN.md with correct syntax
3. [ ] Update IMPLEMENTATION_COMPARISON.md
4. [ ] Fix all test files to use `define [syntax ...]`
5. [ ] Verify examples in SYNTAX.md are correct
6. [ ] Update HEARTBEAT.md to mark as complete

## Template Rewrite Rules

### Simple Macro
```lisp
;; OLD (Scheme)
(define-syntax name
  (syntax-rules ()
    [(_ args ...) template]))

;; NEW (OmniLisp)
(define [syntax name]
  [(name ?args ...) template])
```

### With Literals
```lisp
;; OLD (Scheme)
(define-syntax cond
  (syntax-rules (else)
    [(_ [else body ...]) body ...]
    [(_ [test result] rest ...)
     (if test result (cond rest ...))]))

;; NEW (OmniLisp)
(define [syntax cond]
  [literals else]
  [(cond (else ?body ...)) (do ?body ...)]
  [(cond (?test ?result) ?rest ...)
   (if ?test ?result (cond ?rest ...))])
```

### Gensym
```lisp
;; OmniLisp: same as Scheme
(let ([temp-var (gensym "temp")])
  `(let [,temp-var ...] ...))
```

## Notes

- OmniLisp has **one unified macro system**, not two
- `define` is the universal special form (variables, functions, types, macros)
- Pattern matching is the same as in `match` expressions
- Quasiquote (`` ` ``) still works for code generation
