# OmniLisp C Code Index

## Overview
This document provides a comprehensive index of the C codebase for OmniLisp, organized by directory and functionality.

---

## Directory Structure

```
OmniLisp/
├── runtime/              # C runtime implementation
│   ├── src/             # Runtime source code
│   ├── include/         # Public headers
│   ├── tests/           # Unit tests
│   └── bench/           # Benchmarks
├── csrc/                # Compiler C code
│   ├── codegen/         # Code generation
│   ├── parser/          # Parser implementation
│   └── analysis/        # Static analysis
├── tests/               # Integration tests
└── third_party/         # Third-party libraries
```

---

## Runtime Layer (`runtime/`)

### Core Public API (`runtime/include/omni.h`)
**Lines: 1401**

#### Type System
- **TypeID Constants**: Compile-time type identifiers (TYPE_ID_INT, TYPE_ID_FLOAT, etc.)
- **ObjTag**: Object tags (TAG_INT, TAG_PAIR, TAG_CLOSURE, etc.)
- **Obj Structure**: Core heap-allocated object with fields:
  - `generation`: IPGE generation ID for memory safety
  - `mark`: Reference count
  - `tag`: Object type tag
  - `owner_region`: Owning region pointer

#### Memory Management (CTRR - Compile-Time Region-Based Memory Management)
- **Region-RC System**: Region-based lifetime management
- **IPGE (In-Place Generational Evolution)**: Memory safety via generation IDs
  - Two modes: COMPACT (16-bit gen) and ROBUST (64-bit gen)
  - Full-period LCG for generation evolution
- **Tagged Pointers**: Immediate values (int, char, bool) encoded in pointer bits
- **Scope Tethering**: Vale-style tethered references for scope-bound objects
- **Store Barrier**: `omni_store_repair()` - Automatic lifetime repair at mutation time

#### Region Operations
```c
struct Region* region_create(void);
void region_exit(struct Region* r);
void* transmigrate(void* root, struct Region* src, struct Region* dest);
Obj* alloc_obj_typed(struct Region* r, TypeID type_id);
```

#### Region Lifetime Management
- `omni_obj_region(Obj* o)`: Get owning region of boxed object
- `omni_region_set_lifetime_rank(r, rank)`: Set outlives depth
- `omni_region_outlives(a, b)`: Check if region A outlives region B

#### Object Constructors
- `mk_int()`, `mk_float()`, `mk_char()`: Immediate value constructors
- `mk_pair()`, `mk_box()`, `mk_string()`: Heap object constructors
- `mk_closure()`, `mk_generic()`: Function object constructors
- `mk_array()`, `mk_dict()`, `mk_tuple()`: Collection constructors

#### Reference Counting
```c
void inc_ref(Obj* x);
void dec_ref(Obj* x);
void free_obj(Obj* x);
void defer_decrement(Obj* obj);
```

#### Generic Functions (Multiple Dispatch)
- `mk_generic()`: Create generic function object
- `generic_add_method()`: Add method to generic
- `call_generic()`: Invoke with multiple dispatch
- `MethodInfo`: Single method with specificity score

#### Type System
- `Kind`: Type objects (representing types at runtime)
- `prim_value_to_type()`: Get runtime type of value
- `prim_type_is()`: Type predicate (Julia's isa)
- `prim_union()`: Create union types
- `prim_fn()`: Create function types

#### Concurrency Primitives
- **Channels**: `make_channel()`, `channel_send()`, `channel_recv()`
- **Atoms**: `make_atom()`, `atom_deref()`, `atom_swap()`, `atom_cas()`
- **Threads**: `spawn_thread()`, `thread_join()`
- **Atomics**: `ATOMIC_INC_REF`, `ATOMIC_DEC_REF` macros

#### String Operations
- `prim_string_length()`, `prim_string_split()`, `prim_string_join()`
- `prim_string_replace()`, `prim_string_trim()`
- `prim_string_concat()`, `prim_string_substr()`

#### Regex Primitives
- `prim_re_match()`: First match
- `prim_re_find_all()`: All matches
- `prim_re_split()`: Split by pattern
- `prim_re_replace()`: Replace matches

#### Math Library (Phase 18)
- **Basic**: `prim_add`, `prim_sub`, `prim_mul`, `prim_div`, `prim_mod`, `prim_pow`
- **Trig**: `prim_sin`, `prim_cos`, `prim_tan`, `asin`, `acos`, `atan`, `atan2`
- **Hyperbolic**: `sinh`, `cosh`, `tanh`
- **Exp/Log**: `prim_exp`, `prim_log`, `prim_log10`, `prim_log2`, `prim_sqrt`
- **Rounding**: `floor`, `ceil`, `round`, `trunc`
- **Constants**: `prim_pi`, `prim_e`, `prim_inf`, `prim_nan`
- **Bitwise**: `band`, `bor`, `bxor`, `bnot`, `lshift`, `rshift`

#### Functional Programming
- `prim_pipe()`: Pipe operator
- `prim_compose()`: Function composition
- `prim_apply()`: Apply function to arguments
- `prim_partial()`: Partial application

#### Trampoline (Phase 23)
- `prim_is_bounce()`: Check if thunk
- `prim_bounce()`: Create thunk
- `prim_trampoline()`: Execute thunks

#### Pattern Matching (Pika)
- `prim_match_pattern()`: Match pattern against string
- `prim_compile_pattern()`: Pre-compile pattern
- `prim_pika_parse_grammar()`: Parse grammar spec
- `prim_pika_match()`: Match grammar rule

#### Iterators
- `prim_iterate()`: Create infinite iterator
- `prim_iter_next()`: Get next value
- `prim_take()`: Take n elements

---

### Memory Management (`runtime/src/memory/`)

#### Core Memory Files

**region_core.h** - Region management
- Region struct definition
- `region_create()`, `region_destroy_if_dead()`, `region_exit()`
- `region_retain_internal()`, `region_release_internal()`
- `region_tether_start()`, `region_tether_end()`
- `region_alloc()` - Inline allocation function

**region_core.c** - Region implementation
- Region lifecycle management
- Reference counting for regions
- Tethering support

**transmigrate.h / transmigrate.c** - Object transmigration
- `transmigrate()` - Move objects between regions
- Deep copy of object graphs
- Preserve pointer structure

**region_metadata.h / region_metadata.c** - Type metadata
- Region-level type information
- Allocation strategy optimization
- Inline buffer management

**region_value.h / region_value.c** - Region-aware values
- Region-allocated object constructors
- `mk_int_region()`, `mk_pair_region()`, etc.
- Type-specific allocation

**region_pointer.h** - Pointer utilities
- Pointer tagging/untagging
- Region-safe pointer operations

**arena_core.c** - Arena allocator
- Bump-pointer allocation
- Arena lifecycle

**slot_pool.h / slot_pool.c** - Slot pool
- Fixed-size object pool
- Free list management

**handle.h / handle.c** - Handles
- Handle management system

**continuation.h / continuation.c** - Continuations
- Continuation capture and restore
- Setjmp/longjmp wrapper

---

### Runtime Core (`runtime/src/`)

**runtime.c** - Main runtime (74,060 bytes)
- Object allocation and deallocation
- Primitive implementations
- Runtime initialization
- Global state management

**internal_types.h** - Internal type definitions
- Runtime-private types
- Configuration constants

**types.h** - Public type declarations

**condition.h / condition.c** - Condition system
- Exception handling
- Error propagation
- Condition types

**effect.h / effect.c** - Effect handlers
- Algebraic effects
- Effect handling infrastructure

**restart.h / restart.c** - Restart points
- Restartable exceptions
- Control flow management

**generic.c** - Generic functions
- Multiple dispatch implementation
- Method table management
- Specificity computation

**iterator.c** - Iterators
- Lazy sequence operations
- Iterator protocols

**typed_array.c** - Typed arrays
- Efficient homogeneous collections
- Type-specific operations

**string_utils.c** - String utilities
- String manipulation helpers
- Unicode support

**modules.c** - Module system
- Module import/export
- Namespace management

**piping.c** - Pipe operator
- Function chaining
- Pipe implementation

**trampoline.c** - Trampoline
- Stack-safe recursion
- Bounce/thunk execution

**math_numerics.c** - Math library
- All math primitives
- Trigonometric, exponential, bitwise operations

**regex.c** - Regular expressions
- Regex matching
- Pattern compilation
- Replacement operations

**primitives_specialized.c** - Specialized primitives
- Optimized implementations
- Type-specific operations

---

### Safe Memory Reclamation (`runtime/src/smr/`)

**qsbr.h** - Quiescent-State-Based Reclamation
- QSBR implementation for concurrency
- Thread-safe memory reclamation

---

### Utilities (`runtime/src/util/`)

**hashmap.h / hashmap.c** - Hash table
- String-keyed hash map
- Collision handling
- Resize policy

**dstring.h / dstring.c** - Dynamic strings
- Mutable string buffer
- Efficient concatenation

---

### Atomic Operations (`runtime/include/omni_atomic.h`)
- Atomic reference counting
- Lock-free operations
- C11 atomics support

### Typed Arrays (`runtime/include/typed_array.h`)
- Typed array interface
- Type parameterization
- Collection operations

### Primitives (`runtime/include/primitives_specialized.h`)
- Specialized primitive declarations
- Performance-critical operations

---

## Compiler Layer (`csrc/`)

### Code Generation (`csrc/codegen/`)

**codegen.h** - Code generation interface
- Target-independent codegen API
- Expression compilation

**region_codegen.h** - Region-aware code generation
- Region operations codegen
- Lifetime management code

**spec_codegen.h** - Specialization codegen
- Code specialization based on types
- Monomorphization

**spec_decision.h** - Specialization decisions
- When to specialize
- Cost-benefit analysis

**spec_db.h** - Specialization database
- Track specialized versions
- Cache lookup

**typed_array_codegen.h** - Typed array codegen
- Homogeneous collection compilation
- Type-specific operations

---

### Parser (`csrc/parser/`)

**parser.h** - Parser interface
- Generic parser API
- AST construction

**pika.h** - Pika grammar parser
- PEG parser implementation
- Grammar compilation
- Pattern matching

**pika_c/** - C backend for Pika
- C code generation from Pika grammars
- Optimization passes

---

### Analysis (`csrc/analysis/`)

**analysis.h** - Analysis interface
- Static analysis framework
- Type checking
- Region inference

**type_infer.h** - Type inference
- Type reconstruction
- Constraint solving

**type_env.h** - Type environment
- Type variable binding
- Substitution handling

**type_id.h** - Type ID management
- Type identifier assignment
- Type equality

**region_inference.h** - Region lifetime inference
- Region hierarchy inference
- Outlives relation computation

**scc.c** - Strongly Connected Components
- Cycle detection in type graph
- SCC-based algorithms

**dominator.c** - Dominator tree
- Control flow analysis
- Dominance relationships

**tether_elision.c** - Tether optimization
- Scope tether elimination
- Optimizations for tethered refs

---

### AST (`csrc/ast/`)

**ast.h** - Abstract Syntax Tree
- AST node definitions
- Expression types
- Statement types

---

### Compiler (`csrc/compiler/`)

**compiler.h** - Main compiler
- Compilation pipeline
- Driver logic
- Error reporting

---

## Tests

### Runtime Tests (`runtime/tests/`)
- **Memory tests**: `test_memory.c`, `test_arena.c`
- **Region tests**: `test_region_of_obj.c`, `test_region_unified.c`, `test_iregion.c`
- **RCG tests**: `test_rcg_region.c`, `test_rcg_transmigrate.c`, `test_rcg_ref.c`, `test_rcg_arena.c`
- **Transmigration tests**: `test_transmigration.c`, `test_transmigrate_immediates.c`, `test_immediate_array_transmigrate.c`
- **Concurrency tests**: `test_concurrency.c`, `test_channel_semantics.c`, `test_channel_send_refcount_leak.c`
- **Weak refs**: `test_weak_refs.c`
- **Closures**: `test_closures.c`
- **Deferred cleanup**: `test_deferred.c`
- **Effects**: `test_effect.c`
- **Channels**: `test_channel_send_autorepair.c`, `test_crash_channel.c`
- **Constructors**: `test_constructors.c`
- **Performance**: `test_performance.c`
- **Main test runner**: `test_main.c`
- **Region rank tests**: `test_region_rank_basic.c`
- **Store barrier tests**: `test_store_barrier_merge.c`, `test_store_barrier_rank_autorepair.c`, `test_dict_insert_autorepair.c`
- **Merge tests**: `test_merge_simple.c`, `test_merge_fix2.c`
- **Region RC liveness**: `test_region_rc_liveness.c`
- **Edge cases**: `test_edge_cases_memory.c`
- **Autorepair**: `test_constraint_grow_array_overflow.c`, `test_constraint_strdup_oom.c`, `test_arena_alignment_overflow.c`, `test_linear_region_alignment.c`
- **SCC tests**: `test_scc.c`, `test_scc_refcount_underflow.c`, `test_scc_hashmap_put_failure.c`, `test_scc_refcount_underflow.c`
- **Slot pool tests**: `test_slot_pool.c`, `test_slot_pool_double_free.c`, `test_slot_pool_freelist_overflow.c`, `test_slot_pool_invalid_free.c`
- **Sym concurrency**: `test_sym_concurrency.c`
- **Frame clone**: `test_frame_clone_jmpbuf_issue.c`
- **Pool bounds**: `test_pool_bounds_staleness.c`
- **Type punning**: `test_symmetric_pool_type_punning.c`
- **Verification**: `verify_fixes.c`

### Benchmarks (`runtime/bench/`)
- **Benchmark framework**: `bench_runner.c`
- **Transmigrate**: `bench_transmigration.c`, `bench_batched_transmigrate.c`
- **Thread-local RC**: `bench_thread_local_rc.c`
- **Typed codegen**: `bench_typed_codegen.c`
- **Fat baseline**: `bench_fat_baseline.c`
- **Specialized**: `bench_specialized.c`
- **Inline alloc**: `bench_inline_alloc.c`
- **Tethering**: `bench_tethering.c`
- **Region alloc**: `bench_region_alloc.c`
- **Stress**: `bench_stress.c`
- **Multi-region**: `bench_multiregion.c`
- **Complex**: `bench_complex.c`

### Integration Tests (`tests/`)
- **Pattern cache**: `test_pika_pattern_cache.c`
- **Pattern matching**: `test_omni_pika_match.c`
- **Codegen**: `test_pika_codegen.c`
- **Extract captures**: `test_pika_extract_captures.c`
- **Compile pattern**: `test_omni_compile_pattern.c`
- **Parser tests**: `test_parsing.c`, `test_parsing2.c`, `test_parsing3.c`, `test_parsing4.c`, `test_parsing5.c`, `test_parsing6.c`
- **Grammar tests**: `test_grammar_debug.c`, `test_grammar_debug2.c`
- **Meta parsing**: `test_meta_parse.c`, `test_meta_parse2.c`
- **PEG tests**: `test_simple_peg.c`, `test_debug_peg.c`, `test_peg_gen.c`
- **Pattern tests**: `test_simple_pattern.c`, `test_pattern_debug.c`
- **Compile tests**: `test_compile.c`
- **Debug tests**: `test_debug.c`, `test_embedded.c`
- **Header tests**: `test_header.c`
- **Simple tests**: `test_simple.c`, `test_minimal.c`
- **Layout**: `test_layout.c`
- **String parsing**: `test_string_parsing.c`
- **Repro**: `repro_string_space.c`
- **Tower**: `test_pika_tower.c`

---

## Third-Party Libraries

### Arena (`third_party/arena/`)
- **arena.h**: Bump-pointer arena allocator
- Used by runtime for temporary allocations

### Micro-Arena (`third_party/micro-arena.h/`)
- **micro-arena.h**: Minimal arena implementation
- **test.c**, **example.c**: Usage examples

### SDS (Simple Dynamic Strings) (`third_party/sds/`)
- **sds.h**: Dynamic string library
- **sds.c**: Implementation
- **sdsalloc.h**: Allocator interface

### Linenoise (`third_party/linenoise/`)
- **linenoise.h**: Readline replacement
- **linenoise.c**: Implementation
- Provides line editing and history

### UTHash (`third_party/uthash/`)
- **uthash.h**: Hash table macros
- Provides hash table functionality

### STB DS (`third_party/stb_ds/`)
- **stb_ds.h**: Dynamic array and hash utilities

### Immer (`lib/immer/`)
- **immer_bridge.h**: Bridge to Immer persistent data structures
- Immutable collections library

---

## Key Data Structures

### Obj (Core Object)
```c
struct Obj {
    Generation generation;  // IP generation for safety
    int mark;               // Reference count
    int tag;                // Object type
    int is_pair;
    int scc_id;             // SCC identifier
    unsigned int scan_tag : 31;
    unsigned int tethered : 1;
    Region* owner_region;   // Owning region
    union {
        long i;
        double f;
        struct { Obj *a, *b; };
        void* ptr;
    };
};
```

### Region
```c
struct Region {
    // Defined in region_core.h
    // Contains: arena, refcount, parent pointer, lifetime rank, etc.
};
```

### Closure
```c
struct Closure {
    ClosureFn fn;
    Obj** captures;
    int capture_count;
    int arity;
    const char* name;
};
```

### Generic
```c
typedef struct Generic {
    const char* name;
    MethodInfo* methods;
    int method_count;
} Generic;
```

### MethodInfo
```c
struct MethodInfo {
    Obj** param_kinds;
    int param_count;
    ClosureFn impl;
    int specificity;
    struct MethodInfo* next;
};
```

### Kind
```c
typedef struct Kind {
    char* name;
    Obj** params;
    int param_count;
} Kind;
```

---

## Memory Model Summary

### CTRR (Compile-Time Region-Based Memory Management)
- **No Runtime GC**: All deallocation decisions at compile time
- **Regions**: Hierarchical lifetime scopes
- **Transmigration**: Move objects between regions
- **Store Barrier**: Automatic lifetime repair at mutation
- **Reference Counting**: Region-level RC (per-region, not per-object)

### IPGE (In-Place Generational Evolution)
- **Use-After-Free Detection**: Generation IDs on all objects
- **Two Modes**:
  - COMPACT: 16-bit generation + 48-bit pointer (64-bit total)
  - ROBUST: 64-bit generation + 64-bit pointer (128-bit total)
- **Full-Period LCG**: Bijective evolution, zero collisions

### Immediate Values
- **Tagged Pointers**: 3-bit tag scheme
  - 000: Heap pointer
  - 001: Integer
  - 010: Character
  - 011: Boolean
- **No Heap Allocation**: Primitives encoded in pointer bits

### Scope Tethering
- **Vale-Style**: Tethered references skip generation checks
- **Performance**: Fast path for scope-bound objects

---

## Key Algorithms

### Store Barrier (omni_store_repair)
1. Check if new_value is immediate → store directly
2. Get src_region and dst_region
3. If src == dst → store directly
4. Check if dst outlives src:
   - Yes: Store directly
   - No: Repair via transmigrate or merge
5. Update accounting metrics

### Region Outlives Check
- Uses ancestry relation (parent pointers)
- Ranks alone insufficient (siblings incomparable)
- Walk up parent chain from B to find A

### Transmigration
- Deep copy of object graph
- Recursively copy all reachable objects
- Update pointers in destination region
- Preserve structure

### Multiple Dispatch
- Compute specificity for each method
- Sort methods by specificity
- Select most specific matching method
- Type-based dispatch via Kind objects

---

## Build System

### Compiler: C
- Compiler: GCC/Clang
- Standard: C11
- Platform: Linux (x86-64)
- Line limit: 80 characters
- Indentation: Tabs (8 characters)

### Testing
- Test runner: `runtime/tests/test_main.c`
- Regression tests: 14 tests in `tests.sh`
- Unit tests: Individual test files in `runtime/tests/`

---

## Important Constants

### Type IDs (TypeID enum)
- TYPE_ID_INT = 0
- TYPE_ID_FLOAT = 1
- TYPE_ID_CHAR = 2
- TYPE_ID_PAIR = 3
- TYPE_ID_ARRAY = 4
- TYPE_ID_STRING = 5
- TYPE_ID_SYMBOL = 6
- TYPE_ID_DICT = 7
- TYPE_ID_CLOSURE = 8
- TYPE_ID_BOX = 9
- TYPE_ID_CHANNEL = 10
- TYPE_ID_THREAD = 11
- TYPE_ID_ERROR = 12
- TYPE_ID_ATOM = 13
- TYPE_ID_TUPLE = 14
- TYPE_ID_NAMED_TUPLE = 15
- TYPE_ID_GENERIC = 16
- TYPE_ID_KIND = 17
- TYPE_ID_NOTHING = 18

### Object Tags (ObjTag enum)
- TAG_INT = 1
- TAG_FLOAT = 2
- TAG_CHAR = 3
- TAG_PAIR = 4
- TAG_SYM = 5
- TAG_BOX = 6
- TAG_CLOSURE = 7
- TAG_CHANNEL = 8
- TAG_ERROR = 9
- TAG_ATOM = 10
- TAG_THREAD = 11
- TAG_ARRAY = 12
- TAG_DICT = 13
- TAG_STRING = 14
- TAG_KEYWORD = 15
- TAG_TUPLE = 16
- TAG_NAMED_TUPLE = 17
- TAG_GENERIC = 18
- TAG_KIND = 19
- TAG_NOTHING = 20

### Tagged Pointer Tags
- IMM_TAG_PTR = 0x0 (heap pointer)
- IMM_TAG_INT = 0x1 (immediate integer)
- IMM_TAG_CHAR = 0x2 (immediate character)
- IMM_TAG_BOOL = 0x3 (immediate boolean)

---

## API Design Principles

### Memory Safety
1. Every `malloc`/`calloc` followed by `NULL` check
2. IPGE generation validation for borrowed refs
3. Store barrier for all mutations
4. Region-based lifetime management

### Concurrency
1. Atomic reference counting macros
2. Thread-safe region operations
3. QSBR for memory reclamation
4. Channels and atoms for communication

### Performance
1. Tagged pointers for primitives
2. Scope tethering for fast paths
3. Inline allocation for small objects
4. Type-specific optimized code paths

### Code Style
1. Linux Kernel Style Guide
2. 80-character line limit
3. Tabs for indentation (8 spaces)
4. Over-commenting for clarity

---

## References

### Key Documentation
- `docs/CTRR.md` - Memory model specification
- `runtime/docs/CTRR_TRANSMIGRATION.md` - Transmigration details
- `docs/ADVANCED_REGION_ALGORITHMS.md` - Region algorithms
- `SYNTAX.md` - Lisp syntax
- `AGENTS.md` - Agent operational guidelines

### Phase Implementation
- **Phase 18**: Math and numerics library
- **Phase 19**: Flow constructors (type algebra)
- **Phase 22**: Pipe operator and modules
- **Phase 23**: Trampoline for recursion
- **Phase 24**: Type-metadata-based allocation

### Issues Tracking
- **Issue 1 P1**: region_of(obj) mechanism
- **Issue 2 P4.1**: Region lifetime rank accessors
- **Issue 2 P4.3b**: Region parent/ancestry accessors
- **Issue 2 P4**: Mutation store barrier

---

## Summary Statistics

- **Total C Files**: 100+
- **Total Header Files**: 50+
- **Main API Header**: 1401 lines (omni.h)
- **Core Runtime**: 74KB (runtime.c)
- **Test Coverage**: 14 regression tests
- **Memory Model**: CTRR + IPGE
- **Concurrency**: Atomics, Channels, Atoms, Threads
- **Type System**: Kinds, Generics, Multiple Dispatch, Type Inference
- **Pattern Matching**: Pika PEG parser
- **Math**: Trig, hyperbolic, exp/log, bitwise operations
- **String**: Full string manipulation library
- **Functional**: Map, fold, filter, compose, pipe, partial
- **Control Flow**: Continuations, Effects, Restarts, Trampolines

---

**Index Generated**: January 13, 2026
