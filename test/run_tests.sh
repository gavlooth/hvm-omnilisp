#!/bin/bash
# HVM4 OmniLisp Test Runner
# Run from project root: ./test/run_tests.sh

cd "$(dirname "$0")/.."

HVM4="./hvm4/clang/main"
PASS=0
FAIL=0

echo "=== OmniLisp HVM4 Test Suite ==="
echo ""

# Test that runtime parses
echo -n "Parse runtime.hvm4: "
cat lib/runtime.hvm4 > /tmp/test.hvm4
echo "@main = 0" >> /tmp/test.hvm4
if $HVM4 /tmp/test.hvm4 > /dev/null 2>&1; then
    echo "PASS"
    ((PASS++))
else
    echo "FAIL"
    ((FAIL++))
fi

# Run test files
for test_file in test/test_*.hvm4; do
    name=$(basename "$test_file" .hvm4)
    echo -n "$name: "
    result=$($HVM4 "$test_file" 2>&1)
    if [[ $? -eq 0 ]]; then
        echo "PASS ($result)"
        ((PASS++))
    else
        echo "FAIL"
        echo "  Error: $result"
        ((FAIL++))
    fi
done

echo ""
echo "=== Results: $PASS passed, $FAIL failed ==="
