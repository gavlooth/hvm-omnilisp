#!/bin/bash
# OmniLisp Test Runner
# Runs all test files in the test directory and checks expected outputs

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLANG_DIR="$(dirname "$SCRIPT_DIR")"
OMNILISP="$CLANG_DIR/main"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0
SKIPPED=0

# Count open/close parens in a string (handles brackets too)
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
    local file_passed=0
    local file_failed=0

    echo -e "\n${YELLOW}=== Testing: $(basename "$test_file") ===${NC}"

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
        # Collecting expression or starting to collect
        elif [[ -n "$expected" ]]; then
            # If not blank, add to expression
            if [[ ! "$line" =~ ^[[:space:]]*$ ]]; then
                if [[ $collecting -eq 0 ]]; then
                    collecting=1
                    expr="$line"
                else
                    expr="$expr $line"
                fi
                # Update balance count
                balance=$(count_balance "$expr")
            fi

            # Check if expression is complete (balanced parens)
            if [[ $collecting -eq 1 && $balance -eq 0 && -n "$expr" ]]; then
                # Run the expression using -e flag
                local actual
                actual=$("$OMNILISP" -e "$expr" 2>&1)
                local exit_code=$?

                # Strip "Result:" prefix and trim whitespace
                actual=$(echo "$actual" | sed 's/^Result://' | tr -d '[:space:]')
                expected_trimmed=$(echo "$expected" | tr -d '[:space:]')

                if [[ "$actual" == "$expected_trimmed" ]]; then
                    echo -e "  ${GREEN}PASS${NC}: $test_name"
                    # Show truncated expression if long
                    if [[ ${#expr} -gt 60 ]]; then
                        echo -e "         ${expr:0:60}... => $actual"
                    else
                        echo -e "         $expr => $actual"
                    fi
                    file_passed=$((file_passed + 1))
                    PASSED=$((PASSED + 1))
                else
                    echo -e "  ${RED}FAIL${NC}: $test_name"
                    if [[ ${#expr} -gt 80 ]]; then
                        echo -e "         Expression: ${expr:0:80}..."
                    else
                        echo -e "         Expression: $expr"
                    fi
                    echo -e "         Expected:   $expected"
                    echo -e "         Actual:     $actual"
                    file_failed=$((file_failed + 1))
                    FAILED=$((FAILED + 1))
                fi

                # Reset for next test
                test_name=""
                expected=""
                expr=""
                collecting=0
                balance=0
            fi
        fi
    done < "$test_file"

    echo -e "  File summary: ${GREEN}$file_passed passed${NC}, ${RED}$file_failed failed${NC}"
}

run_multiline_test_file() {
    local test_file="$1"

    echo -e "\n${YELLOW}=== Testing: $(basename "$test_file") ===${NC}"

    # For multiline tests, we run the whole file directly
    local raw_output
    raw_output=$("$OMNILISP" "$test_file" 2>&1)
    local exit_code=$?

    # Extract expected value from the last ;; EXPECT-FINAL: comment
    local expected
    expected=$(grep -E '^[[:space:]]*;;[[:space:]]*EXPECT-FINAL:' "$test_file" | tail -1 | sed 's/.*EXPECT-FINAL:[[:space:]]*//')

    if [[ -z "$expected" ]]; then
        echo -e "  ${YELLOW}SKIPPED${NC}: No EXPECT-FINAL found"
        SKIPPED=$((SKIPPED + 1))
        return
    fi

    # For multiline output, extract the last value after the last comma or the whole thing
    # Output format is "Result:[val1,val2,...,final]" - we want "final"
    local actual
    actual=$(echo "$raw_output" | sed 's/^Result://' | sed 's/.*,//' | tr -d '[:space:][]')
    expected_trimmed=$(echo "$expected" | tr -d '[:space:]')

    if [[ "$actual" == "$expected_trimmed" ]]; then
        echo -e "  ${GREEN}PASS${NC}: $(basename "$test_file")"
        echo -e "         => $actual"
        PASSED=$((PASSED + 1))
    else
        echo -e "  ${RED}FAIL${NC}: $(basename "$test_file")"
        echo -e "         Expected: $expected"
        echo -e "         Actual:   $actual"
        FAILED=$((FAILED + 1))
    fi
}

# Check if omnilisp binary exists
if [[ ! -x "$OMNILISP" ]]; then
    echo -e "${RED}Error: omnilisp binary not found at $OMNILISP${NC}"
    echo "Please build first: cd $CLANG_DIR && make"
    exit 1
fi

echo "OmniLisp Test Runner"
echo "===================="
echo "Using: $OMNILISP"

# Run all test files
for test_file in "$SCRIPT_DIR"/test_*.omni; do
    if [[ -f "$test_file" ]]; then
        # Check if file has multiline tests (contains EXPECT-FINAL)
        if grep -q 'EXPECT-FINAL:' "$test_file"; then
            run_multiline_test_file "$test_file"
        else
            run_test_file "$test_file"
        fi
    fi
done

# Summary
echo -e "\n===================="
echo -e "Total: ${GREEN}$PASSED passed${NC}, ${RED}$FAILED failed${NC}, ${YELLOW}$SKIPPED skipped${NC}"

if [[ $FAILED -gt 0 ]]; then
    exit 1
fi
exit 0
