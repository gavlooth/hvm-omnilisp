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
echo "Running HVM4 print_term coverage tests..."

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

echo "  print_term coverage exercises complete"

exit 0
