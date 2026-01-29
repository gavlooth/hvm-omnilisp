# OmniLisp Complete Test Suite TODO

## Overview
- **Current Coverage**: ~10-15% (23 test files)
- **Target Coverage**: 100%
- **Total Estimated Tests Needed**: ~150+ test files

---

## 1. CORE LANGUAGE (Priority: Critical)

### 1.1 Function Definition (Partially Done)
- [x] `test_define.lisp` - Basic function definition
- [x] `test_define_curried.lisp` - Curried `[x] [y]` syntax
- [x] `test_recursion.lisp` - Recursive functions
- [x] `test_mutual_recursion.lisp` - Mutual recursion
- [ ] `test_define_variadic.lisp` - Rest params `[x .. rest]`
- [ ] `test_define_optional.lisp` - Optional params with defaults
- [ ] `test_define_multiarity.lisp` - Multiple arities in one define
- [ ] `test_lambda.lisp` - `(lambda [x] ...)` syntax
- [ ] `test_lambda_shorthand.lisp` - `(\ [x] ...)` shorthand
- [ ] `test_closure.lisp` - Lexical closures
- [ ] `test_higher_order.lisp` - Functions as arguments/return values

### 1.2 Pattern Matching (Partially Done)
- [x] `test_pattern_literals.lisp` - Literal matching
- [x] `test_pattern_binding.lisp` - Variable binding
- [x] `test_list_patterns.lisp` - List `(h .. t)` patterns
- [x] `test_guards.lisp` - Guard clauses `x & (> x 0)`
- [ ] `test_pattern_nested.lisp` - Nested patterns `((a b) c)`
- [ ] `test_pattern_constructor.lisp` - Constructor patterns `(Point x y)`
- [ ] `test_pattern_array.lisp` - Array patterns `[a b c]`
- [ ] `test_pattern_dict.lisp` - Dict patterns `#{:key val}`
- [ ] `test_pattern_wildcard.lisp` - Wildcard `_` matching
- [ ] `test_pattern_as.lisp` - As-patterns `x @ (h .. t)`
- [ ] `test_pattern_or.lisp` - Or-patterns `(| pat1 pat2)`
- [ ] `test_pattern_exhaustive.lisp` - Exhaustiveness checking

### 1.3 Arithmetic & Math (Partially Done)
- [x] `test_arithmetic.lisp` - Basic `+`, `-`, `*`, `/`
- [x] `test_modulo.lisp` - Modulo `%`
- [ ] `test_math_abs.lisp` - `abs`, `sign`
- [ ] `test_math_minmax.lisp` - `min`, `max`
- [ ] `test_math_sum_product.lisp` - `sum`, `product`
- [ ] `test_math_gcd_lcm.lisp` - `gcd`, `lcm`
- [ ] `test_math_trig.lisp` - `sin`, `cos`, `tan`, `atan`, `atan2`
- [ ] `test_math_exp_log.lisp` - `exp`, `log`, `log10`
- [ ] `test_math_sqrt_pow.lisp` - `sqrt`, `pow`
- [ ] `test_math_floor_ceil.lisp` - `floor`, `ceil`, `round`, `truncate`
- [ ] `test_float.lisp` - Float literals and operations
- [ ] `test_float_special.lisp` - `inf`, `-inf`, `nan` handling
- [ ] `test_bitwise.lisp` - `bit-and`, `bit-or`, `bit-xor`, `bit-not`, `bit-shift`

### 1.4 Comparisons (Partially Done)
- [x] `test_comparison.lisp` - `<`, `>`, `=`, `<=`, `>=`
- [x] `test_not_equal.lisp` - `!=`
- [ ] `test_equality_deep.lisp` - Deep structural equality
- [ ] `test_equality_reference.lisp` - Reference equality `eq?`
- [ ] `test_compare.lisp` - `compare` function (returns -1, 0, 1)

### 1.5 Control Flow (Partially Done)
- [x] `test_do.lisp` - Sequencing
- [x] `test_when.lisp` - `when` conditional
- [x] `test_unless.lisp` - `unless` conditional
- [ ] `test_if.lisp` - Basic `if` expression
- [ ] `test_cond.lisp` - Multi-way `cond`
- [ ] `test_case.lisp` - Value dispatch `case`
- [ ] `test_and_or.lisp` - Short-circuit `and`, `or`
- [ ] `test_not.lisp` - Boolean negation
- [ ] `test_let.lisp` - `let` bindings
- [ ] `test_let_star.lisp` - Sequential `let*`
- [ ] `test_letrec.lisp` - Recursive `letrec`
- [ ] `test_named_let.lisp` - Named let for loops

---

## 2. COLLECTIONS (Priority: High)

### 2.1 List Operations
- [x] `test_quoted_list.lisp` - Quoted list syntax
- [ ] `test_list_cons.lisp` - `cons`, `car`, `cdr`
- [ ] `test_list_head_tail.lisp` - `head`, `tail`, `last`, `init`
- [ ] `test_list_length.lisp` - `length` (expand existing)
- [ ] `test_list_nth.lisp` - `nth`, `first`, `second`, `third`
- [ ] `test_list_append.lisp` - `append`, `concat`
- [ ] `test_list_reverse.lisp` - `reverse`
- [ ] `test_list_take_drop.lisp` - `take`, `drop`
- [ ] `test_list_take_while.lisp` - `take-while`, `drop-while`
- [ ] `test_list_split.lisp` - `split-at`, `split-with`
- [ ] `test_list_flatten.lisp` - `flatten`
- [ ] `test_list_zip.lisp` - `zip`, `zip-with`, `unzip`
- [ ] `test_list_interleave.lisp` - `interleave`, `intersperse`
- [ ] `test_list_partition.lisp` - `partition`, `partition-by`
- [ ] `test_list_group.lisp` - `group-by`, `frequencies`
- [ ] `test_list_sort.lisp` - `sort`, `sort-by`
- [ ] `test_list_unique.lisp` - `nub`, `distinct`, `dedupe`
- [ ] `test_list_find.lisp` - `find`, `find-index`, `index-of`
- [ ] `test_list_member.lisp` - `member?`, `contains?`
- [ ] `test_list_empty.lisp` - `empty?`, `null?`

### 2.2 Higher-Order List Functions
- [x] `test_map.lisp` - Basic map
- [ ] `test_map_indexed.lisp` - `map-indexed`
- [ ] `test_filter.lisp` - `filter`
- [ ] `test_reject.lisp` - `reject`, `remove`
- [ ] `test_foldl.lisp` - `foldl`, `reduce`
- [ ] `test_foldr.lisp` - `foldr`
- [ ] `test_scan.lisp` - `scan`, `reductions`
- [ ] `test_every_some.lisp` - `every?`, `some?`, `none?`
- [ ] `test_count.lisp` - `count`, `count-if`
- [ ] `test_for_each.lisp` - `for-each` (side-effecting map)

### 2.3 Array Operations
- [ ] `test_array_literal.lisp` - Array literal `[1 2 3]`
- [ ] `test_array_get_set.lisp` - `arr-get`, `arr-set`
- [ ] `test_array_length.lisp` - `arr-len`
- [ ] `test_array_slice.lisp` - `arr-slice`
- [ ] `test_array_map.lisp` - `arr-map`, `arr-filter`
- [ ] `test_array_fold.lisp` - `arr-foldl`, `arr-foldr`
- [ ] `test_array_push_pop.lisp` - `arr-push`, `arr-pop`
- [ ] `test_array_concat.lisp` - `arr-concat`
- [ ] `test_array_to_list.lisp` - `arr->list`, `list->arr`

### 2.4 Dict/Map Operations
- [ ] `test_dict_literal.lisp` - Dict literal `#{"a" 1}`
- [ ] `test_dict_get.lisp` - `dict-get`, `dict-get-in`
- [ ] `test_dict_set.lisp` - `dict-set`, `dict-set-in`
- [ ] `test_dict_update.lisp` - `dict-update`, `dict-update-in`
- [ ] `test_dict_remove.lisp` - `dict-remove`, `dict-dissoc`
- [ ] `test_dict_keys_vals.lisp` - `dict-keys`, `dict-vals`, `dict-entries`
- [ ] `test_dict_merge.lisp` - `dict-merge`, `merge-with`
- [ ] `test_dict_select.lisp` - `select-keys`, `rename-keys`
- [ ] `test_dict_has.lisp` - `dict-has?`, `dict-contains?`

### 2.5 Set Operations
- [ ] `test_set_literal.lisp` - Set literal `#{1 2 3}`
- [ ] `test_set_add_remove.lisp` - `set-add`, `set-remove`
- [ ] `test_set_member.lisp` - `set-member?`
- [ ] `test_set_union.lisp` - `set-union`
- [ ] `test_set_intersection.lisp` - `set-intersection`
- [ ] `test_set_difference.lisp` - `set-difference`
- [ ] `test_set_subset.lisp` - `subset?`, `superset?`

---

## 3. ITERATORS & LAZY SEQUENCES (Priority: High)

### 3.1 Generators
- [ ] `test_range.lisp` - `range`, `range-step`
- [ ] `test_iterate.lisp` - `iterate`
- [ ] `test_repeat.lisp` - `repeat`, `repeat-n`
- [ ] `test_cycle.lisp` - `cycle`
- [ ] `test_repeatedly.lisp` - `repeatedly`

### 3.2 Lazy Operations
- [ ] `test_lazy_map.lisp` - Lazy `map`
- [ ] `test_lazy_filter.lisp` - Lazy `filter`
- [ ] `test_lazy_take.lisp` - Lazy `take`, `take-while`
- [ ] `test_lazy_drop.lisp` - Lazy `drop`, `drop-while`
- [ ] `test_lazy_concat.lisp` - Lazy `concat`

### 3.3 Realization
- [ ] `test_collect.lisp` - `collect` (force lazy to list)
- [ ] `test_realize.lisp` - `realize` (force all)
- [ ] `test_into.lisp` - `into` (collect into container)
- [ ] `test_transduce.lisp` - Transducers

---

## 4. PATHS & DATA ACCESS (Priority: High)

### 4.1 Simple Access
- [ ] `test_get.lisp` - `get` from any collection
- [ ] `test_assoc.lisp` - `assoc` (set value)
- [ ] `test_dissoc.lisp` - `dissoc` (remove key)
- [ ] `test_update.lisp` - `update` (apply function)

### 4.2 Nested Access
- [ ] `test_get_in.lisp` - `get-in` nested path
- [ ] `test_assoc_in.lisp` - `assoc-in` nested set
- [ ] `test_update_in.lisp` - `update-in` nested update
- [ ] `test_dissoc_in.lisp` - `dissoc-in` nested remove

### 4.3 Threading Macros
- [ ] `test_thread_first.lisp` - `->` thread-first
- [ ] `test_thread_last.lisp` - `->>` thread-last
- [ ] `test_thread_as.lisp` - `as->` thread-as
- [ ] `test_thread_some.lisp` - `some->`, `some->>`
- [ ] `test_thread_cond.lisp` - `cond->`, `cond->>`

---

## 5. STRINGS (Priority: High)

### 5.1 Basic String Operations
- [ ] `test_str_concat.lisp` - `str`, `str-concat`
- [ ] `test_str_length.lisp` - `str-len`
- [ ] `test_str_get.lisp` - `str-get`, `char-at`
- [ ] `test_str_slice.lisp` - `str-slice`, `substring`
- [ ] `test_str_split.lisp` - `str-split`
- [ ] `test_str_join.lisp` - `str-join`
- [ ] `test_str_replace.lisp` - `str-replace`, `str-replace-all`

### 5.2 String Predicates
- [ ] `test_str_empty.lisp` - `str-empty?`
- [ ] `test_str_starts.lisp` - `str-starts?`, `str-ends?`
- [ ] `test_str_contains.lisp` - `str-contains?`
- [ ] `test_str_compare.lisp` - String comparison

### 5.3 String Transformations
- [ ] `test_str_case.lisp` - `str-upper`, `str-lower`, `str-capitalize`
- [ ] `test_str_trim.lisp` - `str-trim`, `str-trim-left`, `str-trim-right`
- [ ] `test_str_pad.lisp` - `str-pad-left`, `str-pad-right`
- [ ] `test_str_repeat.lisp` - `str-repeat`
- [ ] `test_str_reverse.lisp` - `str-reverse`

### 5.4 String Conversion
- [ ] `test_str_to_int.lisp` - `str->int`, `parse-int`
- [ ] `test_str_to_float.lisp` - `str->float`, `parse-float`
- [ ] `test_int_to_str.lisp` - `int->str`
- [ ] `test_char.lisp` - `char->int`, `int->char`

---

## 6. EFFECTS SYSTEM (Priority: High)

### 6.1 Basic Effects (Partially Done)
- [x] `test_effects.lisp` - Basic handle/perform
- [x] `test_effects_choice.lisp` - Choice effect
- [ ] `test_effects_state.lisp` - State effect (get/put)
- [ ] `test_effects_reader.lisp` - Reader effect (ask)
- [ ] `test_effects_writer.lisp` - Writer effect (tell)
- [ ] `test_effects_exception.lisp` - Exception effect (raise)
- [ ] `test_effects_async.lisp` - Async effect

### 6.2 Effect Handling
- [ ] `test_effects_resume.lisp` - Resumption with values
- [ ] `test_effects_multi_resume.lisp` - Multiple resumptions
- [ ] `test_effects_nested.lisp` - Nested handlers
- [ ] `test_effects_compose.lisp` - Handler composition
- [ ] `test_effects_finally.lisp` - Cleanup/finally behavior

---

## 7. TYPE SYSTEM (Priority: Medium)

### 7.1 Type Predicates (Partially Done)
- [x] `test_type_predicates.lisp` - `int?`, `list?`
- [ ] `test_type_pred_float.lisp` - `float?`, `number?`
- [ ] `test_type_pred_string.lisp` - `string?`, `char?`
- [ ] `test_type_pred_bool.lisp` - `bool?`, `true?`, `false?`
- [ ] `test_type_pred_nil.lisp` - `nil?`, `nothing?`
- [ ] `test_type_pred_fn.lisp` - `fn?`, `callable?`
- [ ] `test_type_pred_coll.lisp` - `array?`, `dict?`, `set?`
- [ ] `test_type_pred_symbol.lisp` - `symbol?`, `keyword?`

### 7.2 Type Annotations
- [ ] `test_type_annotate.lisp` - `{Type}` annotations
- [ ] `test_type_check.lisp` - Runtime type checking
- [ ] `test_type_cast.lisp` - Type casting/coercion

### 7.3 Generic Functions (Multiple Dispatch)
- [ ] `test_defgfun.lisp` - Define generic function
- [ ] `test_defmethod.lisp` - Define method
- [ ] `test_dispatch_single.lisp` - Single dispatch
- [ ] `test_dispatch_multi.lisp` - Multiple dispatch
- [ ] `test_dispatch_hierarchy.lisp` - Type hierarchy dispatch

### 7.4 Custom Types
- [ ] `test_deftype.lisp` - Define custom type
- [ ] `test_deftype_alias.lisp` - Type aliases
- [ ] `test_defstruct.lisp` - Struct definition
- [ ] `test_defrecord.lisp` - Record definition

### 7.5 Advanced Types
- [ ] `test_type_union.lisp` - Union types `(| Int String)`
- [ ] `test_type_intersection.lisp` - Intersection types
- [ ] `test_type_parametric.lisp` - Parametric types `(List Int)`
- [ ] `test_type_constraint.lisp` - Type constraints

---

## 8. MACROS & METAPROGRAMMING (Priority: Medium)

### 8.1 Quasiquote
- [ ] `test_quasiquote.lisp` - Basic quasiquote
- [ ] `test_unquote.lisp` - Unquote `,`
- [ ] `test_splice.lisp` - Splice `,@`
- [ ] `test_nested_qq.lisp` - Nested quasiquote

### 8.2 Syntax Macros
- [ ] `test_define_syntax.lisp` - Define syntax macro
- [ ] `test_syntax_pattern.lisp` - Pattern matching in macros
- [ ] `test_syntax_ellipsis.lisp` - Ellipsis patterns `...`
- [ ] `test_syntax_literals.lisp` - Literal keywords
- [ ] `test_syntax_hygiene.lisp` - Hygienic expansion
- [ ] `test_gensym.lisp` - `gensym` for unique symbols

### 8.3 Macro Expansion
- [ ] `test_macroexpand.lisp` - `macroexpand` one level
- [ ] `test_macroexpand_all.lisp` - `macroexpand-all` recursive

---

## 9. TOWER / META-LEVELS (Priority: Medium)

### 9.1 Basic Tower Operations
- [ ] `test_tower_lift.lisp` - `lift` to code
- [ ] `test_tower_run.lisp` - `run` code
- [ ] `test_tower_reflect.lisp` - `reflect` to higher level
- [ ] `test_tower_reify.lisp` - `reify` to lower level

### 9.2 Staging
- [ ] `test_stage.lisp` - `stage` computation
- [ ] `test_splice_tower.lisp` - `splice` into staged code
- [ ] `test_clambda.lisp` - Staged lambda

### 9.3 Multi-Stage
- [ ] `test_meta_level.lisp` - `meta-level` query
- [ ] `test_eval_meta.lisp` - `eval-meta` at level
- [ ] `test_tower_compose.lisp` - Composing tower operations

---

## 10. REGEX (Priority: Medium)

### 10.1 Regex Matching
- [ ] `test_re_match.lisp` - `re-match` full match
- [ ] `test_re_test.lisp` - `re-test` boolean test
- [ ] `test_re_find.lisp` - `re-find` first match
- [ ] `test_re_find_all.lisp` - `re-find-all` all matches

### 10.2 Regex Operations
- [ ] `test_re_split.lisp` - `re-split`
- [ ] `test_re_replace.lisp` - `re-replace`
- [ ] `test_re_replace_all.lisp` - `re-replace-all`

### 10.3 Regex Syntax
- [ ] `test_re_literals.lisp` - Literal characters
- [ ] `test_re_classes.lisp` - Character classes `[a-z]`
- [ ] `test_re_quantifiers.lisp` - `*`, `+`, `?`, `{n,m}`
- [ ] `test_re_anchors.lisp` - `^`, `$`
- [ ] `test_re_groups.lisp` - Capture groups `(...)`
- [ ] `test_re_alternation.lisp` - Alternation `|`

---

## 11. GRAMMAR / PIKA PARSER (Priority: Medium)

### 11.1 Grammar Definition
- [ ] `test_grammar_rule.lisp` - Define grammar rule
- [ ] `test_grammar_seq.lisp` - Sequence combinator
- [ ] `test_grammar_alt.lisp` - Alternative combinator
- [ ] `test_grammar_rep.lisp` - Repetition `*`, `+`, `?`
- [ ] `test_grammar_neg.lisp` - Negative lookahead

### 11.2 Parsing
- [ ] `test_pika_parse.lisp` - Parse with grammar
- [ ] `test_pika_actions.lisp` - Semantic actions
- [ ] `test_pika_left_recursion.lisp` - Left-recursive grammars

---

## 12. JSON (Priority: Medium)

### 12.1 JSON Parsing
- [ ] `test_json_parse.lisp` - `json-parse`
- [ ] `test_json_parse_null.lisp` - Parse null
- [ ] `test_json_parse_bool.lisp` - Parse true/false
- [ ] `test_json_parse_number.lisp` - Parse numbers
- [ ] `test_json_parse_string.lisp` - Parse strings
- [ ] `test_json_parse_array.lisp` - Parse arrays
- [ ] `test_json_parse_object.lisp` - Parse objects

### 12.2 JSON Stringification
- [ ] `test_json_stringify.lisp` - `json-stringify`
- [ ] `test_json_pretty.lisp` - Pretty printing

### 12.3 JSON Access
- [ ] `test_json_get.lisp` - `json-get`
- [ ] `test_json_type.lisp` - `json-type`

---

## 13. I/O (Priority: Low - requires FFI)

### 13.1 Console I/O
- [ ] `test_print.lisp` - `print`, `println`
- [ ] `test_read_line.lisp` - `read-line`
- [ ] `test_format.lisp` - `format` string formatting

### 13.2 File I/O
- [ ] `test_file_read.lisp` - `read-file`
- [ ] `test_file_write.lisp` - `write-file`
- [ ] `test_file_exists.lisp` - `file-exists?`
- [ ] `test_file_delete.lisp` - `delete-file`
- [ ] `test_file_list.lisp` - `list-dir`
- [ ] `test_file_walk.lisp` - `walk-dir`

---

## 14. CONCURRENCY (Priority: Low - requires FFI)

### 14.1 Fibers
- [ ] `test_fiber_new.lisp` - `fiber-new`
- [ ] `test_fiber_spawn.lisp` - `fiber-spawn`
- [ ] `test_fiber_resume.lisp` - `fiber-resume`
- [ ] `test_fiber_yield.lisp` - `yield`

### 14.2 Channels
- [ ] `test_channel.lisp` - Channel creation
- [ ] `test_channel_send.lisp` - `chan-send`
- [ ] `test_channel_recv.lisp` - `chan-recv`

### 14.3 Async
- [ ] `test_async.lisp` - `async` blocks
- [ ] `test_await.lisp` - `await`
- [ ] `test_async_all.lisp` - `async-all`

---

## 15. NETWORKING (Priority: Low - requires FFI)

### 15.1 TCP
- [ ] `test_tcp_connect.lisp` - `tcp-connect`
- [ ] `test_tcp_listen.lisp` - `tcp-listen`
- [ ] `test_tcp_send.lisp` - `tcp-send`
- [ ] `test_tcp_recv.lisp` - `tcp-recv`

### 15.2 HTTP
- [ ] `test_http_get.lisp` - HTTP GET
- [ ] `test_http_post.lisp` - HTTP POST

---

## 16. DATE/TIME (Priority: Low)

- [ ] `test_datetime_now.lisp` - `datetime-now`
- [ ] `test_datetime_parse.lisp` - `datetime-parse`
- [ ] `test_datetime_format.lisp` - `datetime-format`
- [ ] `test_datetime_add.lisp` - Date arithmetic
- [ ] `test_datetime_diff.lisp` - Date difference

---

## 17. THEOREM PROVING (Priority: Low)

- [ ] `test_theorem_prove.lisp` - Basic proof
- [ ] `test_theorem_induction.lisp` - Inductive proof
- [ ] `test_theorem_sup.lisp` - SUP calculus

---

## 18. PARTIAL EVALUATION (Priority: Low)

- [ ] `test_pe_constant.lisp` - Constant folding
- [ ] `test_pe_inline.lisp` - Function inlining
- [ ] `test_pe_specialize.lisp` - Specialization

---

## 19. DEBUG & TESTING (Priority: Low)

### 19.1 Assertions
- [ ] `test_assert.lisp` - `assert`
- [ ] `test_assert_eq.lisp` - `assert-eq`
- [ ] `test_assert_throws.lisp` - `assert-throws`

### 19.2 Test Framework
- [ ] `test_deftest.lisp` - `deftest`
- [ ] `test_defsuite.lisp` - `defsuite`
- [ ] `test_run_tests.lisp` - `run-tests`

### 19.3 Debugging
- [ ] `test_trace.lisp` - `trace`
- [ ] `test_debug.lisp` - `debug`
- [ ] `test_timed.lisp` - `timed`

---

## Summary

| Category | Tests Needed | Priority |
|----------|-------------|----------|
| Core Language | ~25 | Critical |
| Collections | ~50 | High |
| Iterators | ~15 | High |
| Paths | ~10 | High |
| Strings | ~20 | High |
| Effects | ~10 | High |
| Types | ~20 | Medium |
| Macros | ~10 | Medium |
| Tower | ~10 | Medium |
| Regex | ~15 | Medium |
| Grammar | ~5 | Medium |
| JSON | ~10 | Medium |
| I/O | ~10 | Low |
| Concurrency | ~10 | Low |
| Networking | ~5 | Low |
| DateTime | ~5 | Low |
| Theorem | ~3 | Low |
| Partial Eval | ~3 | Low |
| Debug | ~10 | Low |
| **TOTAL** | **~246** | |

---

## Test File Format

Each test file should follow this format:

```lisp
;; test/test_<feature>.lisp
;; Tests for <feature description>

;; TEST: Basic case
;; EXPECT: expected_value
(expression)

;; TEST: Edge case - empty input
;; EXPECT: expected_value
(expression-with-edge-case)

;; TEST: Error case
;; EXPECT: error
(expression-that-should-fail)
```

---

## Execution Order

1. **Phase 1**: Core Language completion (Critical)
2. **Phase 2**: Collections & Iterators (High)
3. **Phase 3**: Paths, Strings, Effects (High)
4. **Phase 4**: Types, Macros, Tower (Medium)
5. **Phase 5**: Regex, Grammar, JSON (Medium)
6. **Phase 6**: I/O, Concurrency, Networking (Low)
7. **Phase 7**: Remaining features (Low)
