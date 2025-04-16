#!/bin/bash

echo "Running external tests with scripts"
echo ""

echo "Make mysh if have not already"
cd .. && make && cd test

# Create test directory
mkdir -p inputs

# .sh files
# No conditional test so far
TESTS=(
    "Basic"
    "BuiltIn"
    "Redirection"
    "Pipeline"
    "Wildcard"
)

PASSED=0
TOTAL=0

for test in "${TESTS[@]}"; do
    echo ""
    echo "Running test: $test"
    echo "----------------------------------------"
    
    if bash "test${test}.sh"; then
        echo "Test $test PASSED"
        ((PASSED++))
    else
        echo "Test $test FAILED"
    fi
    ((TOTAL++))
    echo ""
done

echo ""
echo "Test Results: $PASSED/$TOTAL passed"
echo ""

# remove temporary files, dont have to but looks nicer
rm -rf inputs
rm -rf *.txt