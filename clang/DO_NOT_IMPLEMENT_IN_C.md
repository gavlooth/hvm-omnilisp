# ⚠️ DO NOT IMPLEMENT IN C INTERPRETER ⚠️

## Critical Directive

**The C interpreter is FROZEN at Phase 2.4.**

## What This Means

### ✅ ALLOWED:
- Bug fixes in existing C interpreter code (main.c)
- Test maintenance
- Running existing tests

### ❌ FORBIDDEN:
- Adding Phase 3+ features to C interpreter
- Modifying main.c for new functionality
- Implementing designs in C code
- Extending C interpreter beyond Phase 2.4

## Implementation Target

**ALL phases 3-8 target HVM4 backend ONLY:**
- Phase 3: Effect system → **HVM4 only**
- Phase 4: Module/Macro/Patterns → **HVM4 only**
- Phase 5: Standard library → **HVM4 only**
- Phase 6-8: Docs/Optimization/Release → **HVM4 only**

## Files to Work With

### HVM4 Implementation:
- `*.hvm4` files
- `omnilisp/compile/_.c` (HVM4 code generator)
- `omnilisp/parse/_.c` (parser)

### DO NOT TOUCH:
- `main.c` evaluation logic (frozen)
- C interpreter runtime (frozen at Phase 2.4)

## Why?

The C interpreter was a **prototype for Phase 1-2**. The real implementation is the **HVM4 backend** which provides:
- Parallel execution
- Optimal reduction
- Native algebraic effects
- True interaction nets

## Status

- **C Interpreter:** Phase 2.4 complete, 481 tests passing, **FROZEN**
- **HVM4 Backend:** Phase 3.1 in progress, active development
