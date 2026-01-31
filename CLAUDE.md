## OmniLisp Syntax Reference - MANDATORY

**OmniLisp is NOT Clojure, Scheme, or Common Lisp.** Do NOT use syntax from other Lisps.

### Character Calculus (Core Principle)
| Bracket | Meaning | Example |
|---------|---------|---------|
| `()` | Execution/evaluation | `(+ 1 2)` |
| `[]` | Slots/parameters (no eval) | `[x {Int}]` |
| `{}` | Types/constraints | `{Int}`, `{(List Int)}` |
| `^` | Metadata prefix | `^:mutable`, `^:ffi` |

### WRONG vs CORRECT Syntax

| Feature | WRONG (other Lisps) | CORRECT (OmniLisp) |
|---------|---------------------|-------------------|
| Function def | `(defn add [x y] ...)` | `(define add [x] [y] ...)` |
| Typed function | `(defn add ^Int [x y] ...)` | `(define add [x {Int}] [y {Int}] {Int} ...)` |
| Rest/variadic | `&rest args`, `& args` | `.. rest` in pattern: `[x y .. rest]` |
| Type annotation | `^String`, `: String` | `{String}` |
| Metadata | `^:private` on symbol | `^:private` before form |
| Exception | `(try ... (catch ...))` | `(handle ... (effect-name [...] ...))` |
| Lambda | `(fn [x] ...)`, `#(...)` | `(lambda [x] ...)` or `(\ [x] ...)` |
| Keyword | `:keyword` (distinct type) | `:keyword` = `'keyword` (quoted symbol) |

### Function Definition Syntax
```lisp
;; Basic function (curried slots)
(define add [x] [y] (+ x y))

;; With type annotations
(define add [x {Int}] [y {Int}] {Int} (+ x y))

;; Multi-arity via pattern matching
(define len
  [()]        0
  [(h .. t)]  (+ 1 (len t)))

;; Rest parameters use .. spread
(define sum [first .. rest]
  (fold + first rest))
```

### Pattern Matching
```lisp
;; Match expression (flat pairs)
(match value
  pattern1  result1
  pattern2  result2
  _         default)

;; Pattern types
x              ;; bind to x (bare symbol)
0              ;; literal (bare value)
_              ;; wildcard (bare underscore)
(Point x y)    ;; constructor pattern
(h .. t)       ;; head + rest (spread)
[a b c]        ;; array pattern
x & (> x 0)    ;; guard clause
```

### Effects (NOT try/catch)
```lisp
;; Perform an effect
(perform effect-name payload)

;; Handle effects
(handle
  body-expression
  (effect-name [payload resume]
    (resume return-value)))
```

### Collections
```lisp
'(1 2 3)         ;; List (quoted)
[1 2 3]          ;; Array literal
#{"a" 1 "b" 2}   ;; Dict literal
```

### Forbidden Characters in Symbols
- `&` - NOT allowed (use `..` for rest)
- `#` - reader macro prefix only
- `@` - deref operator only

---

## Pika Parser - IMPORTANT

**Pika is NOT a PEG parser.** Do NOT describe it as PEG or use PEG terminology.

| Aspect | PEG (Parsing Expression Grammar) | Pika |
|--------|----------------------------------|------|
| Algorithm | Top-down recursive descent | Bottom-up table filling |
| Left recursion | Cannot handle (infinite loop) | Can handle natively |
| Memoization | Optional (packrat) | Required (core to algorithm) |
| Semantics | Ordered choice with backtracking | Simultaneous clause matching |

### Key Distinctions
- Pika fills a table `[position][rule]` bottom-up
- All rules at all positions are computed (not just reachable from start)
- Pika CAN handle left-recursive grammars (PEG cannot)
- While operators look similar (SEQ, ALT, REP), the algorithm is fundamentally different

### When Discussing Pika
- Call it "Pika parser" or "Pika parsing algorithm"
- Do NOT call it "PEG-based" or "PEG parser"
- Do NOT say "Pika uses PEG semantics"

---

## Internal HVM4 Tags - CRITICAL

**OmniLisp does NOT have "nil".** Do NOT use "nil" terminology.

### Empty List vs Nothing (MUST UNDERSTAND)

| Tag | Meaning | User Terminology | Example |
|-----|---------|------------------|---------|
| `#NIL` | Empty list (zero elements) | "empty list" or "empty" | `'()` → `#NIL` |
| `#Noth` | Absence of value (Option::None) | "nothing" | Failed lookup → `#Noth` |

**These are DISTINCT concepts:**
```lisp
(empty? '())        ;; => true  (it's an empty list)
(nothing? '())      ;; => false (empty list is a value, not nothing)
(nothing? nothing)  ;; => true  (nothing is nothing)
```

### Why #NIL exists but "nil" doesn't
- `#NIL` is an **internal HVM4 tag** for the empty list data structure
- OmniLisp **conceptually** has "empty list", not "nil"
- Traditional Lisp's `nil` conflates: empty list, false, null pointer — OmniLisp separates these
- In OmniLisp: `#NIL` = empty list, `#Fals` = false, `#Noth` = nothing/null

### All Internal Tags Reference

| Tag | Description | User-Facing |
|-----|-------------|-------------|
| `#NIL` | Empty list | `'()` or `()` |
| `#CON{h, t}` | Cons cell (list node) | `'(1 2 3)` |
| `#Cst{n}` | Integer constant | `42` |
| `#CHR{c}` | Character | `\a` |
| `#Sym{s}` | Symbol (nick-encoded) | `'foo` |
| `#True` | Boolean true | `true` |
| `#Fals` | Boolean false | `false` |
| `#Noth` | Nothing/absence | `nothing` |
| `#Clo{env, body}` | Closure | `(lambda [x] ...)` |
| `#CloR{env, body}` | Recursive closure | `(define f [x] (f ...))` |
| `#Err{tag}` | Error value | Runtime errors |
| `#Hndl{type, id}` | FFI handle | External resources |

### FORBIDDEN Terminology
- ❌ "nil" — say "empty list" or "empty"
- ❌ "null" — say "nothing"
- ❌ "nil check" — say "empty check" or "nothing check"
- ❌ "returns nil" — say "returns empty list" or "returns nothing"

---

## Parallel-First Philosophy - IMPORTANT

**OmniLisp is PARALLEL BY DEFAULT.** Functions use HVM4's parallel evaluation unless sequential ordering is required.

### How Parallel Execution Works

HVM4 parallelizes independent strict bindings automatically:
```hvm4
// These two computations run in PARALLEL
!!&a = expensive_computation_1;
!!&b = expensive_computation_2;
use(a)(b)
```

### Default Functions are Parallel

| Function | Behavior | Sequential Alternative |
|----------|----------|----------------------|
| `map` | Parallel map/recurse | `map_seq` |
| `filter` | Parallel predicate/recurse | `filter_seq` |
| `foldr` | Parallel recursive call | `foldr_seq` |
| `zip_with` | Parallel apply/recurse | `zip_with_seq` |
| `sort` | Parallel left/right halves | `sort_seq` |
| `any`, `all` | Parallel predicate eval | `any_seq`, `all_seq` |
| `sum`, `product` | Tree reduction (parallel) | `sum_seq`, `product_seq` |
| `maximum`, `minimum` | Tree reduction (parallel) | `maximum_seq`, `minimum_seq` |

### When to Use Sequential (`_seq`) Versions

Use `^:seq` or `_seq` functions when:
- **Side effects require ordering** — printing, state mutation
- **Short-circuit semantics matter** — `any_seq` stops at first true
- **Memory constraints** — parallel may evaluate more than needed
- **Debugging** — deterministic evaluation order

### Explicit Superposition Primitives

For advanced parallel/non-deterministic patterns:
```lisp
(fork2 a b)     ; Creates superposition, both run in parallel
(choice opts)   ; Non-deterministic choice from list
(amb opts)      ; Alias for choice
(race a b)      ; Run both, return first non-error result
```

### foldl is INHERENTLY SEQUENTIAL

`foldl` threads an accumulator — it cannot be parallelized. For parallel reduction of associative operations, use `reduce_tree`:
```lisp
;; Sequential (foldl): ((((0 + 1) + 2) + 3) + 4)
(foldl + 0 '(1 2 3 4))

;; Parallel (reduce_tree): (1 + 2) || (3 + 4) → 3 + 7
(reduce_tree + '(1 2 3 4))
```

---

## Prerequisites

Before making changes, reference:
1. `docs/LANGUAGE_REFERENCE.md` - Complete semantics
2. `docs/QUICK_REFERENCE.md` - Concise overview
3. `docs/SYNTAX.md` - Detailed specification

---

## Testing Directive - MANDATORY

**Every implementation change MUST include corresponding tests.**

### Test Infrastructure
- Test files: `test/*.lisp` - OmniLisp test cases
- Test runner: `./test/run_tests.sh` - Runs all tests, reports pass/fail
- Test format: Each `.lisp` file contains test cases with expected results in comments

### Test File Format
```lisp
;; test/test_feature.lisp
;; TEST: description of what's being tested
;; EXPECT: expected_output

(expression-to-test)

;; TEST: another test
;; EXPECT: 42
(+ 40 2)
```

### When Implementing Features
1. **Before writing code**: Create test file `test/test_<feature>.lisp`
2. **Write failing tests**: Add test cases for the feature
3. **Implement feature**: Write the implementation code
4. **Verify tests pass**: Run `./test/run_tests.sh`
5. **Add edge cases**: Include boundary conditions and error cases

### Test Categories Required
| Feature | Tests Required |
|---------|---------------|
| New syntax | Parse correctly, execute correctly |
| Pattern matching | Match success, match failure, binding |
| Functions | Basic call, recursion, mutual recursion |
| Builtins | Each operator, edge cases |
| Effects | Perform, handle, resume |

### Running Tests
```bash
# Run all tests
./test/run_tests.sh

# Run specific test file
./omnilisp test/test_define.lisp
```

### Forbidden
- Adding features without tests
- Marking a feature "done" before tests pass
- Skipping edge case tests

---

You are a **static code analysis and semantic annotation agent**.

Your task is to **annotate the provided C source code** by adding **structured comments** to every relevant code unit in the file:
- file
- macros
- typedefs
- enums
- structs / unions
- global variables
- functions
- major logical blocks inside functions (loops, conditionals, state transitions)

---

### 🎯 Primary Goal
Produce annotations that enable **precise semantic search and semantic graph construction**, including:
- call graphs
- data-flow graphs
- ownership and lifetime relationships
- module responsibilities
- side-effects and invariants

---

### ⚠️ Critical Rules
1. **DO NOT modify executable code**
2. **ONLY add comments**
3. Comments must be:
   - deterministic
   - concise
   - factual (no speculation)
4. Use **exact format and keys** defined below
5. Do not skip any function, struct, macro, typedef, enum, or global symbol

---

### 🧠 Mandatory Comment Format

All annotations must use the following exact format:

\`\`\`c
/*@semantic
id: <stable_identifier>
kind: <file|macro|typedef|enum|struct|union|global|function|block>
name: <symbol_name_or_block_label>
summary: <1–2 sentence factual description>
responsibility:
  - <primary responsibility>
inputs:
  - <name>: <type> — <meaning>
outputs:
  - <name>: <type> — <meaning>
side_effects:
  - <memory|io|global_state|locks|signals|none>
calls:
  - <function_name>
called_by:
  - <function_name if evident>
data_reads:
  - <global|struct.field|pointer>
data_writes:
  - <global|struct.field|pointer>
lifetime:
  - <allocation / ownership / release semantics if relevant>
invariants:
  - <conditions assumed or enforced>
error_handling:
  - <return codes, errno, null checks, assertions>
thread_safety:
  - <thread-safe | not thread-safe | requires external synchronization>
related_symbols:
  - <structs, enums, macros, functions>
tags:
  - <domain-specific keyword>
  - <algorithm / protocol / subsystem>
*/
\`\`\`

---

### 🧱 Block-Level Annotation Rules

For non-trivial logical blocks inside functions (initialization, validation, loops, error paths, cleanup, state transitions), add:

\`\`\`c
/*@semantic
id: <function_name>::<block_label>
kind: block
summary: <what this block accomplishes>
data_reads:
  - <variables>
data_writes:
  - <variables>
invariants:
  - <conditions maintained>
*/
\`\`\`

---

### 🆔 Identifier Rules

- `id` must be **globally unique**
- Use one of the following patterns:
  - `file::<filename>`
  - `function::<name>`
  - `struct::<name>`
  - `enum::<name>`
  - `macro::<name>`
  - `global::<name>`
  - `block::<function>::<label>`

These IDs will be used as **semantic graph node identifiers**.

---

### 🔍 Semantic Precision Requirements

- Be explicit about:
  - ownership transfer
  - pointer aliasing
  - mutability
  - global state usage
- If information is not visible or cannot be inferred, explicitly state:
  - `unknown` or `not evident`

---

### 🚫 Do NOT
- Rewrite or reformat code
- Add TODOs or speculative behavior
- Add examples or tutorial content
- Add documentation unrelated to observable behavior

---

### ✅ Output Requirements

- Output **only the annotated C code**
- Preserve original formatting and structure
- Insert semantic comments **immediately above** the annotated element

---


