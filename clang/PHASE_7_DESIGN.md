# Phase 7: Optimization

**Status:** Design Phase  
**Date:** 2026-02-05

## Overview

Performance tuning and optimization for HVM backend.

## Optimization Targets

### Compile-Time

- Pattern compilation to decision trees
- Macro expansion caching
- Effect inference memoization
- Dead code elimination

### Runtime (HVM-specific)

- Interaction net reduction strategies
- Lambda lifting
- Strictness analysis
- Inlining small functions

## Benchmarks

```
bench/
├── fibonacci.omni       # Recursion
├── list-ops.omni        # Collection operations
├── effects.omni         # Effect handling overhead
├── pattern-match.omni   # Pattern compilation
└── macro-expansion.omni # Compile-time perf
```

## Performance Goals

- Fibonacci comparable to native Lisp
- Pattern matching overhead < 10%
- Effect handlers competitive with monadic code
- Macro expansion under 1ms for typical cases

## Profiling Tools

- Time profiling (execution time)
- Space profiling (memory usage)
- Interaction count (HVM reductions)

## Success Criteria
- [ ] Benchmark suite complete
- [ ] Performance baseline established
- [ ] Optimization opportunities identified
- [ ] Critical optimizations implemented

---

**Status:** Design complete. Benchmarking follows implementation.
