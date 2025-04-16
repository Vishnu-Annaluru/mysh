#!/bin/bash

echo "TESTING REDIRECTION"

cat > inputs/redirection.txt << EOF
echo "This is a test file" > test.txt
cat test.txt
echo "Adding more" > test2.txt
cat < test.txt
cat < test2.txt
rm test.txt test2.txt
EOF

cd .. && ./mysh test/inputs/redirection.txt > test/redirectionOutput.txt 2>&1

if grep -q "This is a test file" test/redirectionOutput.txt && \
   grep -q "Adding more" test/redirectionOutput.txt; then
    exit 0
else
    cat test/redirectionOutput.txt
    exit 1
fi
