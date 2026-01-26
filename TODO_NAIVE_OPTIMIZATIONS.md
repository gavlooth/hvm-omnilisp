# REVIEWED:NAIVE Optimization Tasks

This document tracks remaining `REVIEWED:NAIVE` blocks in the codebase.

**Status:** Most critical optimizations have been completed. See git commit `ab7848f` for details.

---

## Completed Optimizations

The following have been optimized with StrMap (wyhash-based O(1) lookup):

### Infrastructure
- StrMap implementation: `csrc/util/strmap.{c,h}`, `runtime/src/util/strmap.{c,h}`

### Runtime (Phase 1)
- **1.27-1.28** generic.c: Dispatch cache + arity-grouped method lists
- **1.24-1.26** condition.c: Name/ID hash maps + per-condition slot maps
- **modules.c**: Module registry StrMap
- **4.3-4.4** string_utils.c: Pre-calculated lengths avoiding O(n²) strcat

### Analysis (Phase 2)
- **1.8-1.11** analysis.c: Parallel hash maps for var/escape/owner lookups
- **1.15** type_env.c: Per-scope binding hash maps
- **1.16, 2.3-2.4** region_inference.c: VIG node hash map + neighbor hash sets
- **2.9-2.10** analysis.c: Hash-based liveness sets

### Codegen (Phase 3)
- **1.1-1.4** codegen.c: Symbol map, capture set, function definition tracking
- **1.5-1.7** spec_db.c: Two-level func_name -> signature chain lookup
- **5.1** profile.c: O(n log n) qsort replacing O(n²) bubble sort

### Safety (Phase 4)
- **3.1-3.3** typed_array_codegen.c: Dynamic allocation for dimension buffers
- **3.5** analysis.c: Dynamic allocation for parametric instance names
- **4.1** pika.c: Cached strlen avoiding O(n²) loop

### Macro System (Phase 5)
- **1.34** macro.c: StrMap for O(1) macro lookup by name

---

## Remaining Tasks (Lower Priority)

### Category 1: Linear Lookup (P3 - Rare Code Paths)

#### 1.17 analysis/component.c:11 - `find_or_create_component`
**Current:** Linked list traversal for ComponentInfo lookup
**Impact:** Called during SCC component building
**Mitigation:** Array indexed by scc_id (IDs are typically small integers).

#### 1.18 analysis/tether_elision.c:12 - `find_matching_exit`
**Current:** Linear scan through tether points for matching exit
**Impact:** Called during tether elision optimization
**Mitigation:** Store entry/exit pairs in a map keyed by region_id.

#### 1.19 analysis/static_sym.c:12 - CFG node lookup by position
**Current:** Linear scan through CFG nodes checking position ranges
**Impact:** Called to map source positions to CFG nodes
**Mitigation:** Build interval tree or sorted array with binary search.

#### 1.20-1.21 ast/ast.c:562,601 - Dict operations
**Current:** Linear scan through dict key-value pairs
**Impact:** Called for every dictionary access
**Note:** Already using hash buckets, may need optimization review.

#### 1.22-1.23 ast/ast.c:626,637 - User type field access
**Current:** Linear scan through field names
**Impact:** Called for every field access on user types
**Mitigation:** For small structs (<8 fields), linear is fine. For larger, use field name hash table.

#### 1.29-1.31 macro/pattern.c, template.c - Pattern/template bindings
**Current:** Linked list traversal for binding lookup
**Impact:** Called during macro expansion
**Mitigation:** Hash table keyed by variable name.

#### 1.32 macro/hygiene.c:46 - Reserved symbol check
**Current:** Linear scan through static reserved symbols array (45 symbols)
**Impact:** Called for every symbol during hygiene processing
**Mitigation:** Use perfect hash (gperf) or static hash set.

#### 1.33 macro/hygiene.c:94 - `omni_macro_lookup_rename`
**Current:** Linked list traversal for rename entries
**Impact:** Called during hygienic macro expansion
**Mitigation:** Hash table keyed by (original_name, mark) pair.

---

### Category 2: O(n²) Algorithms (Potentially Inherent)

#### 2.1-2.2 analysis/region_inference.c:224,281 - Variable interaction graph
**Current:** Nested loops connecting all variable pairs - O(n²)
**Impact:** Called to build variable interaction graph
**Note:** May be inherently O(n²) if all pairs must be connected. Consider:
- Lazy edge creation
- Implicit complete subgraph representation

#### 2.5-2.7 analysis/type_env.c, type_infer.c - Union type operations
**Current:** O(n²) nested loops for deduplication and comparison
**Impact:** Called when creating/comparing union types
**Mitigation:** Sort members and compare linearly, or use canonical form with hash.

#### 2.8 analysis/analysis.c:2094 - `cfg_node_add_use`
**Current:** O(n) duplicate check before adding use
**Impact:** Called for every variable use in CFG
**Mitigation:** Hash set for uses per CFG node.

#### 2.11 analysis/analysis.c:4272 - Ownership edge update
**Current:** Linear scan through edges to find matching edge
**Impact:** Called during ownership graph updates
**Mitigation:** Hash table keyed by (from_type, from_field) pair.

---

### Category 3: Fixed Buffer (Low Risk)

#### 3.4 runtime/collections.c:929 - List to array conversion
**Current:** Hardcoded 1024 element limit
**Impact:** Truncates lists longer than 1024 elements
**Mitigation:** Two-pass (count then allocate) or dynamic reallocation.

---

### Category 4: String Operations (Minor)

#### 4.2 codegen/spec_db.c:128 - In-place space removal
**Current:** In-place modification with pointer manipulation
**Impact:** Minor - already O(n)
**Status:** REVIEWED:NAIVE - acceptable performance

#### 4.5 runtime/string_utils.c:486 - `prim_string_last_index_of`
**Current:** Reverse linear scan from end
**Impact:** O(n×m) where n=string length, m=substring length
**Mitigation:** Use Boyer-Moore for large strings. Current approach acceptable for small strings.

#### 4.6-4.7 runtime/runtime.c:3348,3420 - Type name construction
**Current:** Loop building type name with strcat
**Impact:** O(n²) for many union members/parameters
**Mitigation:** Pre-calculate total length, single allocation, pointer arithmetic.

---

### Category 6: Miscellaneous (Low Priority)

#### 6.1 codegen/codegen.c:6375 - `omni_codegen_emit_region_exits`
**Current:** Placeholder/stub implementation
**Impact:** Region exits not emitted optimally
**Mitigation:** Implement proper region exit tracking during codegen.

#### 6.4 analysis/component.c:44 - CFG node by position lookup
**Current:** Linear scan checking position ranges
**Impact:** O(n) per lookup
**Mitigation:** Interval tree or binary search.

#### 6.5 parser/pika_c/pika.c:1283 - `handle_precedence`
**Current:** Complex precedence handling logic
**Impact:** Called during grammar processing
**Mitigation:** Review algorithm - may need architectural change.

#### 6.6 macro/pattern.c:328 - Ellipsis binding accumulation
**Current:** Linear search for target binding in accumulated results
**Impact:** O(n×m) for n iterations × m bindings
**Mitigation:** Use hash table for bindings during accumulation.

#### 6.7 macro/template.c:40 - `collect_template_vars`
**Current:** Recursive traversal with duplicate accumulation
**Impact:** O(n²) due to duplicate checking
**Mitigation:** Use hash set for seen variables.

#### 6.8 runtime/regex_compile.c:1142 - `pika_regex_replace`
**Current:** String building for regex replacement
**Impact:** Depends on implementation details
**Mitigation:** Use StringBuilder pattern with growth strategy.

---

## Priority Summary

| Priority | Status | Description |
|----------|--------|-------------|
| P0 | **DONE** | Critical hot paths (generic dispatch, var tracking, type lookup) |
| P1 | **DONE** | Compile-time performance (codegen, spec_db, liveness) |
| P2 | **DONE** | Safety fixes (buffer overflows, strlen loop) |
| P3 | Remaining | Rare code paths (condition, macro internals) |

---

## Hash Infrastructure

The codebase now uses `StrMap` - a string-keyed hash map using wyhash:

```c
#include "util/strmap.h"

StrMap* map = strmap_new();
strmap_put(map, "key", value);
void* v = strmap_get(map, "key");
bool has = strmap_contains(map, "key");
strmap_remove(map, "key");
strmap_free(map);
```

For new optimizations, use this infrastructure rather than introducing new hash libraries.
