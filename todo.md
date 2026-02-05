# OmniLisp HVM4 Integration - Complete Implementation Todo

## Current Status: ~65-70% Complete

| Component | Status | Gap |
|-----------|--------|-----|
| Parser/Compiler | 100% | - |
| Core @omni_eval | 39% | 242 of 398 handlers missing |
| Type System | 40% | No unification, inference, compile-time checking |
| Effects System | 60% | No multi-shot continuations, effect polymorphism |
| Module System | 30% | No file loading, circular detection |
| Test Coverage | 15% | Need 150+ additional tests |

---

## PHASE 1: Core Runtime Handlers

### 1.1 String Operations

**File to modify:** `lib/runtime.hvm4` (add to `@omni_eval`)

**Handler implementation pattern:**
```hvm4
#SLen: λ&str.
  (λ&s. #FFI{#sym_str_len, #CON{s, #NIL}})(@omni_eval(menv)(str))
```

| Handler | OmniLisp Function | Description | Test File |
|---------|-------------------|-------------|-----------|
| `#SLen` | `str-len` | String length | `test_str_length.omni` (EXISTS) |
| `#SEmp` | `str-empty?` | Empty check | `test_str_length.omni` (EXISTS) |
| `#SChc` | `str-char-at` | Get char at index | `test_str_get.omni` (EXISTS) |
| `#SSub` | `str-slice` | Substring | `test_str_slice.omni` (EXISTS) |
| `#SUpR` | `str-upper` | Uppercase | `test_str_case.omni` (EXISTS) |
| `#SLwR` | `str-lower` | Lowercase | `test_str_case.omni` (EXISTS) |
| `#SCap` | `str-capitalize` | Capitalize first | NEW: `test_str_capitalize.omni` |
| `#STrm` | `str-trim` | Trim whitespace | `test_str_trim.omni` (EXISTS) |
| `#SInd` | `str-index-of` | Find substring | NEW: `test_str_index.omni` |
| `#SCnt` | `str-contains?` | Contains check | `test_str_contains.omni` (EXISTS) |
| `#SSta` | `str-starts?` | Prefix check | `test_str_starts.omni` (EXISTS) |
| `#SEnd` | `str-ends?` | Suffix check | NEW: `test_str_ends.omni` |
| `#SSpl` | `str-split` | Split by delim | `test_str_split.omni` (EXISTS) |
| `#SJoi` | `str-join` | Join with delim | `test_str_join.omni` (EXISTS) |
| `#SRpl` | `str-replace` | Replace occurrences | `test_str_replace.omni` (EXISTS) |
| `#SRev` | `str-reverse` | Reverse string | `test_str_reverse.omni` (EXISTS) |
| `#SRep` | `str-repeat` | Repeat n times | `test_str_repeat.omni` (EXISTS) |
| `#SPad` | `str-pad` | Pad to length | NEW: `test_str_pad.omni` |
| `#SCmp` | `str-compare` | Lexicographic compare | NEW: `test_str_compare.omni` |
| `#Fmts` | `format` | String formatting | NEW: `test_str_format.omni` |

**New test files to create:**
- [ ] `clang/test/test_str_index.omni`
- [ ] `clang/test/test_str_ends.omni`
- [ ] `clang/test/test_str_capitalize.omni`
- [ ] `clang/test/test_str_pad.omni`
- [ ] `clang/test/test_str_compare.omni`
- [ ] `clang/test/test_str_format.omni`

---

### 1.2 IO Operations

**Files to modify:**
- `lib/runtime.hvm4` (handlers)
- `clang/omnilisp/ffi/handle.c` (FFI implementations)

| Handler | OmniLisp Function | Description | Test File |
|---------|-------------------|-------------|-----------|
| `#RdFl` | `read-file` | Read entire file | NEW: `test_io_read_file.omni` |
| `#WrFl` | `write-file` | Write file | NEW: `test_io_write_file.omni` |
| `#ApFl` | `append-file` | Append to file | NEW: `test_io_append_file.omni` |
| `#Exst` | `file-exists?` | Check existence | NEW: `test_io_exists.omni` |
| `#IsDr` | `dir?` | Is directory | NEW: `test_io_dir.omni` |
| `#MkDr` | `mkdir` | Create directory | NEW: `test_io_mkdir.omni` |
| `#LsDr` | `list-dir` | List directory | NEW: `test_io_listdir.omni` |
| `#DlFl` | `delete-file` | Delete file | NEW: `test_io_delete.omni` |
| `#RnFl` | `rename-file` | Rename/move file | NEW: `test_io_rename.omni` |
| `#CpFl` | `copy-file` | Copy file | NEW: `test_io_copy.omni` |
| `#GtEv` | `getenv` | Get env variable | NEW: `test_io_env.omni` |
| `#StEv` | `setenv` | Set env variable | NEW: `test_io_env.omni` |
| `#Prnt` | `print` | Print without newline | Likely exists |
| `#PrnL` | `println` | Print with newline | Likely exists |
| `#RdLn` | `read-line` | Read line from stdin | NEW: `test_io_readline.omni` |

**New test files to create:**
- [ ] `clang/test/test_io_read_file.omni`
- [ ] `clang/test/test_io_write_file.omni`
- [ ] `clang/test/test_io_append_file.omni`
- [ ] `clang/test/test_io_exists.omni`
- [ ] `clang/test/test_io_dir.omni`
- [ ] `clang/test/test_io_mkdir.omni`
- [ ] `clang/test/test_io_listdir.omni`
- [ ] `clang/test/test_io_delete.omni`
- [ ] `clang/test/test_io_rename.omni`
- [ ] `clang/test/test_io_copy.omni`
- [ ] `clang/test/test_io_env.omni`
- [ ] `clang/test/test_io_readline.omni`

---

### 1.3 DateTime Operations

**File to modify:** `lib/runtime.hvm4`

**DateTime representation:**
```hvm4
#Dt{year, month, day, hour, min, sec, nsec}
#Dur{secs, nsecs}
```

| Handler | OmniLisp Function | Description | Test File |
|---------|-------------------|-------------|-----------|
| `#DtNw` | `datetime-now` | Current datetime | `test_datetime_add.omni` (EXISTS) |
| `#DtTs` | `datetime->timestamp` | To Unix timestamp | NEW: `test_datetime_convert.omni` |
| `#DtFt` | `timestamp->datetime` | From timestamp | NEW: `test_datetime_convert.omni` |
| `#DtPr` | `datetime-parse` | Parse from string | NEW: `test_datetime_parse.omni` |
| `#DtFm` | `datetime-format` | Format to string | NEW: `test_datetime_format.omni` |
| `#DtAd` | `datetime-add` | Add duration | `test_datetime_add.omni` (EXISTS) |
| `#DtSb` | `datetime-sub` | Subtract duration | NEW: `test_datetime_sub.omni` |
| `#DtDf` | `datetime-diff` | Difference | NEW: `test_datetime_diff.omni` |
| `#DtYr` | `datetime-year` | Get year | NEW: `test_datetime_parts.omni` |
| `#DtMo` | `datetime-month` | Get month | NEW: `test_datetime_parts.omni` |
| `#DtDy` | `datetime-day` | Get day | NEW: `test_datetime_parts.omni` |
| `#DtHr` | `datetime-hour` | Get hour | NEW: `test_datetime_parts.omni` |
| `#DtMi` | `datetime-minute` | Get minute | NEW: `test_datetime_parts.omni` |
| `#DtSc` | `datetime-second` | Get second | NEW: `test_datetime_parts.omni` |

**New test files to create:**
- [ ] `clang/test/test_datetime_convert.omni`
- [ ] `clang/test/test_datetime_parse.omni`
- [ ] `clang/test/test_datetime_format.omni`
- [ ] `clang/test/test_datetime_sub.omni`
- [ ] `clang/test/test_datetime_diff.omni`
- [ ] `clang/test/test_datetime_parts.omni`

---

### 1.4 JSON Operations

**File to modify:** `lib/runtime.hvm4`

**JSON representation:**
- JSON objects -> OmniLisp Dict `#Dict{...}`
- JSON arrays -> OmniLisp Array `#Arr{len, data}`
- JSON null -> `#JNul{}`

| Handler | OmniLisp Function | Description | Test File |
|---------|-------------------|-------------|-----------|
| `#JPrs` | `json-parse` | Parse JSON string | NEW: `test_json_parse.omni` |
| `#JStr` | `json-stringify` | Serialize to JSON | NEW: `test_json_stringify.omni` |
| `#JArr` | `json-array?` | Is JSON array | NEW: `test_json_types.omni` |
| `#JObj` | `json-object?` | Is JSON object | NEW: `test_json_types.omni` |
| `#JNul` | `json-null` | JSON null value | NEW: `test_json_types.omni` |

**New test files to create:**
- [ ] `clang/test/test_json_parse.omni`
- [ ] `clang/test/test_json_stringify.omni`
- [ ] `clang/test/test_json_types.omni`

---

### 1.5 Collection Operations

**Files to modify:** `lib/runtime.hvm4`, `lib/prelude.hvm4`

#### Array Operations

| Handler | OmniLisp Function | Description |
|---------|-------------------|-------------|
| `#AGe` | `array-get` | Get element at index |
| `#ASe` | `array-set` | Functional set (returns new array) |
| `#ALe` | `array-length` | Array length |
| `#Slce` | `slice` | Generic slice |
| `#Take` | `take` | Take first n elements |
| `#Drop` | `drop` | Drop first n elements |

**Test file:** NEW `clang/test/test_array_ops.omni`

#### Dict Operations

| Handler | OmniLisp Function | Description |
|---------|-------------------|-------------|
| `#DGe` | `dict-get` | Get value by key |
| `#DSe` | `dict-set` | Set key-value pair |
| `#Diss` | `dissoc` | Remove key |
| `#Keys` | `keys` | Get all keys |
| `#Vals` | `vals` | Get all values |
| `#Ents` | `entries` | Get all key-value pairs |
| `#Mrge` | `merge` | Merge dictionaries |
| `#Asoc` | `assoc` | Associate key with value |

**Test file:** NEW `clang/test/test_dict_ops.omni`

#### List Operations

| Handler | OmniLisp Function | Description |
|---------|-------------------|-------------|
| `#Nth` | `nth` | Get nth element |
| `#Last` | `last` | Get last element |
| `#Init` | `init` | All but last |
| `#Flat` | `flatten` | Flatten nested lists |
| `#Intl` | `interleave` | Interleave two lists |
| `#Intp` | `interpose` | Insert separator |
| `#Part` | `partition` | Partition into groups |
| `#Grby` | `group-by` | Group by function |
| `#Sort` | `sort` | Sort (default compare) |
| `#SrtB` | `sort-by` | Sort by function |
| `#Dist` | `distinct` | Remove duplicates |
| `#Freq` | `frequencies` | Count occurrences |
| `#Find` | `find` | Find first match |
| `#Any` | `any?` | Any match predicate |
| `#All` | `all?` | All match predicate |
| `#Remv` | `remove` | Remove matching elements |
| `#Zip` | `zip` | Zip lists together |

**Test file:** NEW `clang/test/test_list_advanced.omni`

**New test files to create:**
- [ ] `clang/test/test_array_ops.omni`
- [ ] `clang/test/test_dict_ops.omni`
- [ ] `clang/test/test_list_advanced.omni`

---

### 1.6 Iterator Operations

**Files to modify:** `lib/runtime.hvm4`, `lib/iterators.hvm4`

| Handler | OmniLisp Function | Description |
|---------|-------------------|-------------|
| `#Iter` | `iter` | Create iterator from collection |
| `#Rang` | `range` | Range iterator |
| `#Done` | (internal) | Iterator exhausted marker |
| `#IMap` | `lazy-map` | Lazy map |
| `#IFlt` | `lazy-filter` | Lazy filter |
| `#ITkn` | `take` | Take first n |
| `#IDrp` | `drop` | Drop first n |
| `#ITkW` | `take-while` | Take while predicate |
| `#IDrW` | `drop-while` | Drop while predicate |
| `#IZip` | `zip` | Zip iterators |
| `#IChn` | `chain` | Chain iterators |
| `#IEnm` | `enumerate` | Add indices |
| `#IFMp` | `flat-map` | Map and flatten |
| `#IFld` | `iter-fold` | Fold over iterator |
| `#IFnd` | `iter-find` | Find first match |
| `#IAny` | `iter-any?` | Any match? |
| `#IAll` | `iter-all?` | All match? |
| `#INth` | `iter-nth` | Get nth element |

**New test file to create:**
- [ ] `clang/test/test_iterator_ops.omni`

---

### 1.7 Math Operations

**File to modify:** `lib/runtime.hvm4`

| Handler | OmniLisp Function | Description |
|---------|-------------------|-------------|
| `#Sqrt` | `sqrt` | Square root (EXISTS) |
| `#Pow` | `pow` | Power (EXISTS) |
| `#MAbs` | `abs` | Absolute value |
| `#Flor` | `floor` | Floor |
| `#MCei` | `ceil` | Ceiling |
| `#Rond` | `round` | Round |
| `#MSin` | `sin` | Sine |
| `#MCos` | `cos` | Cosine |
| `#MTan` | `tan` | Tangent |
| `#MAsn` | `asin` | Arc sine |
| `#MAcs` | `acos` | Arc cosine |
| `#MAtn` | `atan` | Arc tangent |
| `#MExp` | `exp` | e^x |
| `#MLog` | `log` | Natural log |
| `#Rand` | `random` | Random 0-1 |

**New test file to create:**
- [ ] `clang/test/test_math_ops.omni`

---

## PHASE 2: Type System Completion

### 2.1 Type Unification

**New file to create:** `lib/type_unify.hvm4`

**Implementation tasks:**
- [ ] Create `@unify(type_a, type_b)` function
  - Returns `#USuc{subst}` on success
  - Returns `#UFal{reason}` on failure
- [ ] Handle structural unification for:
  - `#TApp{base, params}` - type application
  - `#TFun{args, ret}` - function types
  - `#TFunE{args, ret, effects}` - function types with effects
  - `#TVar{name}` - type variables
- [ ] Implement occurs check (prevent infinite types)
- [ ] Create `@apply_subst(subst, type)` to apply substitution

**Test files to create:**
- [ ] `clang/test/test_type_unify.omni`
- [ ] `clang/test/test_type_infer_basic.omni`

---

### 2.2 Constraint Generation & Solving

**New file to create:** `lib/type_infer.hvm4`

**Implementation tasks:**
- [ ] Create `@generate_constraints(expr, expected_type)`
  - Returns list of `#TConstr{t1, t2}` constraints
- [ ] Create `@solve_constraints(constraints)`
  - Returns `#USuc{subst}` or `#UFal{constraint}`
  - Use worklist algorithm
- [ ] Handle constraints from:
  - Function application
  - Let bindings
  - Match expressions
  - If expressions

---

### 2.3 Compile-Time Type Checking

**File to modify:** `clang/omnilisp/compile/_.c`

**Implementation tasks:**
- [ ] Add `TypeContext` struct to track types during compilation
- [ ] Add `omni_type_check(OmniEmit *e, Term t, Type expected)` function
- [ ] Call type checking before `omni_emit_term` when types annotated
- [ ] Emit warnings/errors for type mismatches
- [ ] Add command-line flag to enable type checking: `--type-check`

**Test files to create:**
- [ ] `clang/test/test_type_check_pass.omni`
- [ ] `clang/test/test_type_check_fail.omni`

---

### 2.4 Generic Type Inference

**Implementation tasks:**
- [ ] Implement let generalization
  - Create type schemes `#TScm{vars, type}`
  - Generalize free type variables not in environment
- [ ] Implement scheme instantiation at use sites
- [ ] Add bidirectional type checking:
  - Synthesis mode: infer type from expression
  - Checking mode: verify expression has expected type

**Test files to create:**
- [ ] `clang/test/test_type_infer_poly.omni`
- [ ] `clang/test/test_type_generalize.omni`

---

### 2.5 Higher-Kinded Types

**New file to create:** `lib/kinds.hvm4`

**Implementation tasks:**
- [ ] Create kind system:
  - `#Kind{arity}` - kind with arity (0 = *, 1 = * -> *)
  - `#KFun{from, to}` - kind function
- [ ] Create `@kind_of(type)` to get kind of type
- [ ] Add type constructor variables `#TConVar{name, kind}`
- [ ] Update unification to check kinds

**Test files to create:**
- [ ] `clang/test/test_hkt_functor.omni`
- [ ] `clang/test/test_hkt_monad.omni`

---

## PHASE 3: Effects System Completion

### 3.1 Effect Row Types

**File to modify:** `lib/types.hvm4`

**Implementation tasks:**
- [ ] Extend effect row representation:
  - `#ERws{effects}` - concrete effect row
  - `#ERVar{name}` - effect row variable
  - `#ERCon{effect, rest}` - effect row constructor
- [ ] Create effect row operations:
  - `@effect_row_union(row1, row2)`
  - `@effect_row_subset(row1, row2)`
  - `@effect_row_diff(row1, row2)`
- [ ] Add subtyping rules for effect rows

**Test files to create:**
- [ ] `clang/test/test_effect_row_types.omni`
- [ ] `clang/test/test_effect_subtyping.omni`

---

### 3.2 Effect Polymorphism

**File to modify:** `lib/runtime.hvm4`

**Implementation tasks:**
- [ ] Extend constraint generation for effects
- [ ] Implement effect row unification
- [ ] Track effect propagation through call chains
- [ ] Allow effect variables in function signatures:
  ```lisp
  (define pure-map [f {(fn [a] b ^:effects E)}] [xs] ^:effects E
    ...)
  ```

**Test files to create:**
- [ ] `clang/test/test_effect_polymorphism.omni`
- [ ] `clang/test/test_effect_propagation.omni`

---

### 3.3 Effect Composition

**File to modify:** `lib/runtime.hvm4`

**Implementation tasks:**
- [ ] Implement `@omni_compose_handlers(h1, h2)` - sequential composition
- [ ] Implement `@omni_merge_handlers(h1, h2)` - parallel composition
- [ ] Implement `@omni_lift_handler(h, layer)` - handler lifting
- [ ] Add effect alias support:
  ```lisp
  (define ^:effect-alias IO [Print Read Write])
  ```

**Test files to create:**
- [ ] `clang/test/test_effect_compose.omni`
- [ ] `clang/test/test_effect_aliases.omni`
- [ ] `clang/test/test_handler_transformers.omni`

---

## PHASE 4: Module System Completion

### 4.1 File-Based Module Loading

**Files to modify:**
- `clang/main.c`
- `lib/runtime.hvm4`
- `clang/omnilisp/ffi/handle.c`

**Implementation tasks:**

**In `clang/main.c`:**
- [ ] Add `omni_find_module_file(const char *name)` function
  - Search order: current dir, `./lib/`, `../lib/`, `OMNI_MODULE_PATH`
  - File extensions: `.omni`, `.ol`
- [ ] Add `omni_load_module(const char *name)` function
  - Check if already loaded in registry
  - Find and read file
  - Parse with `omni_parse()`
  - Evaluate module definition
  - Return module reference

**In `clang/omnilisp/ffi/handle.c`:**
- [ ] Add `load_module` FFI handler:
  ```c
  if (strcmp(name, "load_module") == 0) {
      const char *mod_name = omni_string_to_c(arg);
      return omni_load_module(mod_name);
  }
  ```

**In `lib/runtime.hvm4`:**
- [ ] Update `@omni_process_import` to trigger file load:
  ```hvm4
  @omni_process_import = λ&menv. λ&import.
    λ{
      #Impt: λ&mod_name. λ&bindings.
        (λ&mod.
          λ{
            #Noth: // Not found - try loading from file
              (λ&loaded. ...)(@omni_load_module_ffi(mod_name))
            _: λ&u_. ... // Already loaded
          }(mod)
        )(@omni_lookup_module(mod_name))
    }(import)
  ```

**Test file to create:**
- [ ] `clang/test/test_module_file.omni`

---

### 4.2 Circular Dependency Detection

**File to modify:** `lib/runtime.hvm4`

**Implementation tasks:**
- [ ] Add module loading stack:
  ```hvm4
  @omni_module_loading_stack = #NIL

  @omni_module_loading_push = λ&name. ...
  @omni_module_loading_pop = λ_. ...
  @omni_module_is_loading = λ&name. ...
  ```
- [ ] Check for cycles before loading
- [ ] Return `#Err{#sym_CircularDependency, name}` if detected
- [ ] Pop from stack after successful load

**Test file to create:**
- [ ] `clang/test/test_module_circular.omni` (should produce error)

---

### 4.3 Qualified Access

**Files to modify:**
- `clang/omnilisp/parse/_.c`
- `lib/runtime.hvm4`

**Implementation tasks:**

**In parser (`clang/omnilisp/parse/_.c`):**
- [ ] Detect `.` in symbol parsing
- [ ] Split into module name and symbol name
- [ ] Generate AST node: `#QAcc{module_nick, symbol_nick}`

**In runtime (`lib/runtime.hvm4`):**
- [ ] Add `#QAcc` handler to `@omni_eval`:
  ```hvm4
  #QAcc: λ&mod_name. λ&sym_name.
    (λ&mod.
      λ{
        #Noth: #Err{#sym_ModuleNotFound, mod_name}
        #Mod: λ&name. λ&mod_env.
          @omni_module_lookup_nick(sym_name)(mod_env)
        _: λ&u_. #Err{#sym_InvalidModule}
      }(mod)
    )(@omni_lookup_module(mod_name))
  ```

**Test file to create:**
- [ ] `clang/test/test_module_qualified.omni`

---

### 4.4 Module Search Paths

**File to modify:** `clang/main.c`

**Implementation tasks:**
- [ ] Add command-line option `-L path` or `--lib-path path`
- [ ] Parse `OMNI_MODULE_PATH` environment variable (colon-separated)
- [ ] Store paths in global array `OMNI_MODULE_PATHS[]`
- [ ] Use in `omni_find_module_file()` search order

---

### 4.5 Re-exports

**Files to modify:**
- `clang/omnilisp/parse/_.c`
- `lib/runtime.hvm4`

**Syntax:**
```lisp
(module Utils
  (export :all-from Math)      ;; Re-export all from Math
  (export :from Math sin cos)  ;; Re-export specific symbols
  ...)
```

**Implementation tasks:**
- [ ] Parser recognizes `:all-from` and `:from` in export list
- [ ] Export struct becomes `#Expt{direct_exports, reexports}`
- [ ] `@omni_filter_exports` handles re-export resolution

**Test file to create:**
- [ ] `clang/test/test_module_reexport.omni`

---

## PHASE 5: Macro System & Gensym

### 5.1 Gensym FFI Implementation

**Files to modify:**
- `clang/omnilisp/ffi/handle.c`
- `lib/syntax.hvm4`

**Implementation in C (`clang/omnilisp/ffi/handle.c`):**
```c
#include <stdatomic.h>

static atomic_uint_fast64_t OMNI_GENSYM_COUNTER = 0;

Term omni_ffi_gensym(Term prefix) {
    uint64_t id = atomic_fetch_add(&OMNI_GENSYM_COUNTER, 1);
    // Get prefix nick
    u32 prefix_nick = term_val(HEAP[term_val(prefix)]);
    // Create unique nick: prefix + "_G" + id
    char buf[256];
    omni_nick_to_string(prefix_nick, buf, sizeof(buf));
    char unique[512];
    snprintf(unique, sizeof(unique), "%s_G%lu", buf, id);
    u32 unique_nick = omni_nick(unique);
    return omni_sym(unique_nick);
}
```

**Add to FFI dispatch:**
```c
if (strcmp(name, "gensym") == 0) {
    return omni_ffi_gensym(args);
}
```

**Update `lib/syntax.hvm4`:**
```hvm4
// Replace static counter with FFI call
@omni_gensym = λprefix.
  #FFI{#sym_gensym, #CON{prefix, #NIL}}
```

**Test file to create:**
- [ ] `clang/test/test_gensym.omni`

---

## Test File Template

```lisp
;; test_feature_name.omni - Tests for feature description

;; TEST: basic functionality
;; EXPECT: expected_value
(expression-to-test)

;; TEST: edge case
;; EXPECT: expected_value
(edge-case-expression)

;; TEST: error handling
;; EXPECT: #Err{error_tag}
(should-error-expression)
```

For stateful tests:
```lisp
;; test_stateful_feature.omni

(define helper [x] (+ x 1))

;; EXPECT-FINAL: 6
(helper 5)
```

---

## Running Tests

```bash
# Run all tests
cd clang/test && ./run_tests.sh

# Check specific test
./clang/main -e "(str-len \"hello\")"
```

---

## Error Handling Convention

All handlers should use consistent error representation:

```hvm4
#Err{error_tag}              // Simple error
#Err{error_tag, details}     // Error with details
```

**Standard error tags:**
| Tag | Meaning |
|-----|---------|
| `#sym_type_error` | Wrong argument type |
| `#sym_arity_error` | Wrong number of arguments |
| `#sym_index_out_of_bounds` | Index outside valid range |
| `#sym_file_not_found` | File does not exist |
| `#sym_permission_denied` | Insufficient permissions |
| `#sym_io_error` | General IO error |
| `#sym_parse_error` | Failed to parse input |
| `#sym_not_implemented` | Feature not yet implemented |
| `#sym_CircularDependency` | Circular module import |
| `#sym_ModuleNotFound` | Module not found |

---

## Summary Checklist

### Phase 1: Core Handlers
- [ ] String operations (6 new test files)
- [ ] IO operations (12 new test files)
- [ ] DateTime operations (6 new test files)
- [ ] JSON operations (3 new test files)
- [ ] Collection operations (3 new test files)
- [ ] Iterator operations (1 new test file)
- [ ] Math operations (1 new test file)

### Phase 2: Type System
- [ ] `lib/type_unify.hvm4` (new file)
- [ ] `lib/type_infer.hvm4` (new file)
- [ ] Compile-time type checking in `compile/_.c`
- [ ] `lib/kinds.hvm4` (new file)
- [ ] 6 new test files

### Phase 3: Effects
- [ ] Effect row types in `lib/types.hvm4`
- [ ] Effect polymorphism in `lib/runtime.hvm4`
- [ ] Effect composition operators
- [ ] 5 new test files

### Phase 4: Module System
- [ ] File loading in `clang/main.c`
- [ ] Circular detection in `lib/runtime.hvm4`
- [ ] Qualified access in parser + runtime
- [ ] Module search paths
- [ ] Re-exports
- [ ] 5 new test files

### Phase 5: Macros
- [ ] Gensym FFI in `clang/omnilisp/ffi/handle.c`
- [ ] Update `lib/syntax.hvm4`
- [ ] 1 new test file

**Total new test files needed: ~48**
**Estimated completion: 12 weeks**
