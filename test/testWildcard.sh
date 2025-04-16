#!/bin/bash

echo "TESTING WILDCARD"

cat > inputs/wildcard.txt << EOF
echo "Creating test files..."
touch test1.txt test2.txt test3.log
echo "Listing .txt files:"
ls test*.txt
echo "Listing .log files:"
ls test*.log
echo "Listing all test files:"
ls test*
rm test1.txt test2.txt test3.log
EOF

cd .. && ./mysh test/inputs/wildcard.txt > test/wildcardOutput.txt 2>&1

# Check if wildcard expansion worked
if grep -q "test1.txt" test/wildcardOutput.txt && grep -q "test2.txt" test/wildcardOutput.txt && grep -q "test3.log" test/wildcardOutput.txt; then
    exit 0
else
    cat test/wildcardOutput.txt
    exit 1
fi
