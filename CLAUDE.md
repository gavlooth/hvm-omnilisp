## OmniLisp Syntax Reference - MANDATORY

**OmniLisp is NOT Clojure, Scheme, or Common Lisp.** Do NOT use syntax from other Lisps.

### Character Calculus (Core Principle)
| Bracket | Meaning | Example |
|---------|---------|---------|
| `()` | Execution/evaluation | `(+ 1 2)` |
| `[]` | Slots/parameters (no eval) | `[x {Int}]` |
| `{}` | Types/constraints | `{Int}`, `{List {Int}}` |
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
;; Match expression
(match value
  [pattern1]  result1
  [pattern2]  result2
  [_]         default)

;; Patterns
[x]              ;; bind to x
[x y]            ;; destructure pair/tuple
[(Point x y)]    ;; constructor pattern
[h .. t]         ;; head + rest (spread)
[x & (> x 0)]    ;; guard clause
[_]              ;; wildcard
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

## Prerequisites

Before making changes, reference:
1. `docs/LANGUAGE_REFERENCE.md` - Complete semantics
2. `docs/QUICK_REFERENCE.md` - Concise overview
3. `docs/SYNTAX.md` - Detailed specification

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


