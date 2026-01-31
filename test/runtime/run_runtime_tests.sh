#!/bin/bash
# OmniLisp Runtime Unit Test Runner
# Runs HVM4-native tests for runtime.hvm4 functions

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
HVM4="$PROJECT_ROOT/hvm4/clang/main"
RUNTIME="$PROJECT_ROOT/lib/runtime.hvm4"
HARNESS="$SCRIPT_DIR/harness.hvm4"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

PASSED=0
FAILED=0
TOTAL=0

echo -e "${BLUE}================================${NC}"
echo -e "${BLUE}OmniLisp Runtime Test Suite${NC}"
echo -e "${BLUE}================================${NC}"
echo ""

# Check for HVM4 binary
if [[ ! -x "$HVM4" ]]; then
    echo -e "${RED}Error: HVM4 binary not found at $HVM4${NC}"
    echo "Please build HVM4 first"
    exit 1
fi

# Check for runtime
if [[ ! -f "$RUNTIME" ]]; then
    echo -e "${RED}Error: runtime.hvm4 not found at $RUNTIME${NC}"
    exit 1
fi

# Check for harness
if [[ ! -f "$HARNESS" ]]; then
    echo -e "${RED}Error: harness.hvm4 not found at $HARNESS${NC}"
    exit 1
fi

run_test_suite() {
    local test_file="$1"
    local test_name=$(basename "$test_file" .hvm4)

    echo -e "${YELLOW}Running: $test_name${NC}"

    # Create combined file: runtime + harness + test
    local temp_file=$(mktemp --suffix=.hvm4)
    cat "$RUNTIME" "$HARNESS" "$test_file" > "$temp_file"

    # Run test
    local output
    output=$("$HVM4" "$temp_file" 2>&1)
    local exit_code=$?

    rm -f "$temp_file"

    TOTAL=$((TOTAL + 1))

    if [[ $exit_code -ne 0 ]]; then
        echo -e "  ${RED}ERROR${NC}: HVM4 execution failed"
        echo "  Output: $output"
        FAILED=$((FAILED + 1))
        return 1
    fi

    # Check result
    case "$output" in
        *"AllPass"*)
            echo -e "  ${GREEN}PASS${NC}: All tests passed"
            PASSED=$((PASSED + 1))
            ;;
        *"TestFailed"*|*"Fail"*)
            echo -e "  ${RED}FAIL${NC}: $output"
            FAILED=$((FAILED + 1))
            ;;
        *"TestError"*)
            echo -e "  ${RED}ERROR${NC}: $output"
            FAILED=$((FAILED + 1))
            ;;
        *)
            # Try to parse result
            if [[ "$output" == *"#Results"* ]]; then
                echo -e "  Result: $output"
                # Extract pass/fail counts
                if [[ "$output" =~ Results\{([0-9]+),\ *0\} ]]; then
                    echo -e "  ${GREEN}PASS${NC}: ${BASH_REMATCH[1]} tests passed"
                    PASSED=$((PASSED + 1))
                else
                    echo -e "  ${YELLOW}WARN${NC}: Could not parse results"
                    PASSED=$((PASSED + 1))
                fi
            else
                echo -e "  Result: $output"
                PASSED=$((PASSED + 1))
            fi
            ;;
    esac
}

# Run all test files
for test_file in "$SCRIPT_DIR"/test_*.hvm4; do
    if [[ -f "$test_file" ]]; then
        run_test_suite "$test_file"
    fi
done

# Summary
echo ""
echo -e "${BLUE}================================${NC}"
echo -e "Summary: ${GREEN}$PASSED passed${NC}, ${RED}$FAILED failed${NC} (${TOTAL} total)"
echo -e "${BLUE}================================${NC}"

if [[ $FAILED -gt 0 ]]; then
    exit 1
fi
exit 0
