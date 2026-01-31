#!/bin/bash
# OmniLisp Coverage Test Runner
# Runs all test files to gather coverage data

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OMNILISP="${1:-$SCRIPT_DIR/../main-cov}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

PASSED=0
FAILED=0
TOTAL=0

# Check if omnilisp binary exists
if [[ ! -x "$OMNILISP" ]]; then
    echo -e "${RED}Error: omnilisp binary not found at $OMNILISP${NC}"
    exit 1
fi

echo "OmniLisp Coverage Test Runner"
echo "============================="
echo "Using: $OMNILISP"

# Count open/close parens in a string
count_balance() {
    local str="$1"
    local open=0
    local i
    for (( i=0; i<${#str}; i++ )); do
        c="${str:$i:1}"
        case "$c" in
            '('|'[') ((open++)) ;;
            ')'|']') ((open--)) ;;
        esac
    done
    echo $open
}

run_test_file() {
    local test_file="$1"
    local test_name=""
    local expected=""
    local expr=""
    local collecting=0
    local balance=0

    while IFS= read -r line || [[ -n "$line" ]]; do
        # Extract TEST description
        if [[ "$line" =~ ^[[:space:]]*\;\;[[:space:]]*TEST:[[:space:]]*(.*)$ ]]; then
            test_name="${BASH_REMATCH[1]}"
        # Extract EXPECT value
        elif [[ "$line" =~ ^[[:space:]]*\;\;[[:space:]]*EXPECT:[[:space:]]*(.*)$ ]]; then
            expected="${BASH_REMATCH[1]}"
            expr=""
            collecting=0
            balance=0
        # Skip comment lines when collecting
        elif [[ "$line" =~ ^[[:space:]]*\;\; ]]; then
            continue
        # Skip empty lines when not yet collecting
        elif [[ "$line" =~ ^[[:space:]]*$ && $collecting -eq 0 ]]; then
            continue
        # Collecting expression
        elif [[ -n "$expected" ]]; then
            if [[ ! "$line" =~ ^[[:space:]]*$ ]]; then
                if [[ $collecting -eq 0 ]]; then
                    collecting=1
                    expr="$line"
                else
                    expr="$expr $line"
                fi
                balance=$(count_balance "$expr")
            fi

            # Check if expression is complete
            if [[ $collecting -eq 1 && $balance -eq 0 && -n "$expr" ]]; then
                TOTAL=$((TOTAL + 1))
                local actual
                actual=$("$OMNILISP" -e "$expr" 2>&1)
                actual=$(echo "$actual" | sed 's/^Result://' | tr -d '[:space:]')
                expected_trimmed=$(echo "$expected" | tr -d '[:space:]')

                if [[ "$actual" == "$expected_trimmed" ]]; then
                    PASSED=$((PASSED + 1))
                else
                    FAILED=$((FAILED + 1))
                fi

                test_name=""
                expected=""
                expr=""
                collecting=0
                balance=0
            fi
        fi
    done < "$test_file"
}

run_multiline_test_file() {
    local test_file="$1"
    TOTAL=$((TOTAL + 1))

    local raw_output
    raw_output=$("$OMNILISP" "$test_file" 2>&1)

    local expected
    expected=$(grep -E '^[[:space:]]*;;[[:space:]]*EXPECT-FINAL:' "$test_file" | tail -1 | sed 's/.*EXPECT-FINAL:[[:space:]]*//')

    if [[ -z "$expected" ]]; then
        return
    fi

    local actual
    actual=$(echo "$raw_output" | sed 's/^Result://' | sed 's/.*,//' | tr -d '[:space:][]')
    expected_trimmed=$(echo "$expected" | tr -d '[:space:]')

    if [[ "$actual" == "$expected_trimmed" ]]; then
        PASSED=$((PASSED + 1))
    else
        FAILED=$((FAILED + 1))
    fi
}

# Run all test files
for test_file in "$SCRIPT_DIR"/test_*.omni; do
    if [[ -f "$test_file" ]]; then
        echo -n "."
        if grep -q 'EXPECT-FINAL:' "$test_file"; then
            run_multiline_test_file "$test_file"
        else
            run_test_file "$test_file"
        fi
    fi
done

echo ""
echo ""
echo "============================="
echo -e "Results: ${GREEN}$PASSED passed${NC}, ${RED}$FAILED failed${NC}, Total: $TOTAL"

# ==============================================================================
# HVM4 print_term coverage
# Exercise HVM4's print_term function for term.c coverage
# ==============================================================================
echo ""
echo "Running OmniLisp print_term coverage tests..."

# Test expressions that exercise different term types and print_term code paths
PRINT_EXPRS=(
    "1"                           # NUM
    "'foo"                        # SYM
    "true"                        # True
    "false"                       # False
    "'()"                         # NIL
    "'(1 2 3)"                    # List/CON
    "[1 2 3]"                     # Array
    "(lambda [x] x)"              # Closure
    "(lambda [x] (+ x x))"        # Closure with dup
    "(+ 1 2)"                     # Simple computation
    "(if true 1 2)"               # Conditional
    "(match 1 1 :one 2 :two)"     # Pattern match
    "(let [x 10] x)"              # Let binding
    "(do 1 2 3)"                  # Sequence
    "\\a"                          # Character
    "\"hello\""                   # String
)

for expr in "${PRINT_EXPRS[@]}"; do
    "$OMNILISP" -T -e "$expr" > /dev/null 2>&1
done

echo "  OmniLisp print_term coverage exercises complete"

# ==============================================================================
# HVM4 native tests for raw term printing coverage
# Run HVM4 tests that exercise LAM, APP, SUP, DUP, MOV, MAT term types
# ==============================================================================
echo ""
echo "Running HVM4 native tests for term.c coverage..."

HVM4_COV="${SCRIPT_DIR}/../hvm4-cov"
HVM4_TEST_DIR="${SCRIPT_DIR}/../../hvm4/test"

if [[ -x "$HVM4_COV" && -d "$HVM4_TEST_DIR" ]]; then
    # Comprehensive HVM4 tests for term.c coverage
    HVM4_TESTS=(
        # === Autodup tests (DUP terms, multi-char names) ===
        "autodup_1.hvm4"
        "autodup_2.hvm4"
        "autodup_3.hvm4"
        "autodup_4.hvm4"
        "autodup_deep.hvm4"
        "autodup_let.hvm4"
        "autodup_many.hvm4"
        "autodup_mat_lam.hvm4"
        "autodup_nested_lam.hvm4"
        "autodup_nightmare.hvm4"
        "autodup_mixed.hvm4"
        "autodup_bound_vars.hvm4"
        "autodup_strict.hvm4"

        # === DUP terms ===
        "dup_affine_valid.hvm4"
        "dup_fresh.hvm4"

        # === Lambda tests (LAM/APP/VAR terms) ===
        "lambda_eval.hvm4"
        "lam_dup_dyn.hvm4"
        "lam_dup_static.hvm4"
        "lam_dup_sup.hvm4"

        # === Match tests (MAT/SWI - print_mat_name) ===
        "match_0.hvm4"
        "match_1.hvm4"
        "match_2.hvm4"
        "match_3.hvm4"
        "print_mat.hvm4"
        "print_mat_default.hvm4"
        "print_mat_lst.hvm4"
        "print_mat_nat.hvm4"
        "print_swi.hvm4"
        "sugar_mat_lst.hvm4"
        "sugar_mat_nat.hvm4"

        # === Collapse tests (SUP terms, print_alpha_name, print_lam_name) ===
        "collapse_0.hvm4"
        "collapse_1.hvm4"
        "collapse_2.hvm4"
        "collapse_3.hvm4"
        "collapse_4.hvm4"
        "collapse_5.hvm4"
        "collapse_6.hvm4"
        "collapse_7.hvm4"
        "collapse_8.hvm4"

        # === Sugar tests (print_ctr: nat, char, string, list) ===
        "sugar_chr.hvm4"
        "sugar_cons.hvm4"
        "sugar_lst.hvm4"
        "sugar_nat.hvm4"
        "sugar_nat_plus.hvm4"
        "sugar_nil.hvm4"
        "sugar_str.hvm4"

        # === Primitive ops (OP2 printing) ===
        "prim_add.hvm4"
        "prim_sub.hvm4"
        "prim_mul.hvm4"
        "prim_div.hvm4"
        "prim_mod.hvm4"
        "prim_and.hvm4"
        "prim_or.hvm4"
        "prim_xor.hvm4"
        "prim_lsh.hvm4"
        "prim_rsh.hvm4"
        "prim_not.hvm4"
        "prim_eq.hvm4"
        "prim_ne.hvm4"
        "prim_lt.hvm4"
        "prim_le.hvm4"
        "prim_gt.hvm4"
        "prim_ge.hvm4"
        "prim_sp0.hvm4"
        "prim_sp1.hvm4"
        "prim_sup.hvm4"

        # === RED/AND/OR tests (comparison terms) ===
        "red_add_stuck_a.hvm4"
        "red_add_stuck_b.hvm4"
        "red_and_ff.hvm4"
        "red_and_tf.hvm4"
        "red_and_tt.hvm4"
        "red_and_stuck_a.hvm4"
        "red_and_stuck_b.hvm4"
        "red_dbl.hvm4"
        "red_dbl_stuck.hvm4"
        "red_dup.hvm4"
        "red_foo.hvm4"
        "red_foo2.hvm4"
        "red_id.hvm4"
        "red_not.hvm4"
        "red_not2.hvm4"
        "red_prd.hvm4"
        "red_prd_direct.hvm4"
        "red_simple.hvm4"
        "red_sum.hvm4"
        "red_sum_stuck.hvm4"

        # === Unscoped tests (UNS term, unscoped vars) ===
        "unscoped.hvm4"
        "unscoped_callcc.hvm4"
        "unscoped_mut_ref.hvm4"
        "unscoped_nested.hvm4"
        "unscoped_queue.hvm4"
        "unscoped_var.hvm4"
        "scoped_var.hvm4"
        "scoped_var_both.hvm4"
        "snf_unscoped.hvm4"

        # === Unicode tests (UTF-8 printing) ===
        "unicode_char.hvm4"
        "unicode_cjk.hvm4"
        "unicode_emoji.hvm4"
        "unicode_greek.hvm4"
        "unicode_mixed.hvm4"
        "unicode_string.hvm4"

        # === Church numerals (multi-char names from deep lambdas) ===
        "c2_not_t.hvm4"
        "cadd_c1_c1_not.hvm4"
        "cadd_c2_c2.hvm4"
        "cadd_c4_c4.hvm4"
        "cmul_c4_c2.hvm4"
        "cmul_c4_c4.hvm4"
        "cnot_not.hvm4"
        "cnot_true.hvm4"
        "snf_c2_k2.hvm4"

        # === Misc tests ===
        "sum_4.hvm4"
    )

    for test in "${HVM4_TESTS[@]}"; do
        if [[ -f "$HVM4_TEST_DIR/$test" ]]; then
            "$HVM4_COV" "$HVM4_TEST_DIR/$test" -s > /dev/null 2>&1
        fi
    done
    echo "  HVM4 native tests complete"
else
    echo "  Skipping HVM4 tests (hvm4-cov not found or test dir missing)"
fi

exit 0
