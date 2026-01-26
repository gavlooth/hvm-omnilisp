# Unwired Features Report
**Date:** 2026-01-13
**Status:** Analysis of "Unwired" Code

"Unwired" refers to features that are partially implemented (often in C) or planned, but not yet exposed to the OmniLisp runtime or compiler for general use.

## 1. Multiple Dispatch (Phase 19)
- **Status:** Partially implemented in C backend.
- **Evidence:** 
    - `csrc/codegen/codegen.c`: Contains `omni_codegen_dispatch_call`, trampoline generation, and "Multiple Dispatch Support" comments.
    - `tests/unwired_features.omni`: Tests explicitly marked `(UNWIRED)` fail for dispatch and multi-arity.
- **Missing:** Final wiring of the dispatch logic to the function definition syntax in the compiler.

## 2. Pika Parser API
- **Status:** Core logic exists; Runtime API missing.
- **Evidence:**
    - `csrc/parser/pika_core.c`: Functional Pika parser implementation.
    - `tests/unwired_features.omni`: `pika-match` primitive is missing.
- **Missing:** Binding the C `pika_parse` functions to a Lisp primitive like `(pika-match ...)`.

## 3. Parametric Subtyping
- **Status:** Analysis logic exists.
- **Evidence:** 
    - `tests/unwired_features.omni`: `{List Int}` vs `{List Any}` subtype checks are unwired.
    - `csrc/analysis/analysis.c`: Contains type inference and specificity logic.

## 4. Iterators & Sequences
- **Status:** Missing primitives.
- **Evidence:** `tests/unwired_features.omni` shows `iterate` and `take` are undefined.

## 5. Other
- **Character Literals:** Reader support missing (`#\newline`).
- **Format Strings:** Reader support missing (`#fmt"..."`).
- **Scientific Computing:** BLAS/Torch integration planned but untouched.

## Summary
The codebase has significant "dark code" in `csrc/codegen` and `csrc/analysis` that supports advanced features (dispatch, types), but these are not yet reachable from the OmniLisp surface language.
