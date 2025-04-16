#!/bin/bash

echo "TESTING BUILT-IN"

cat > inputs/builtIn.txt << EOF
cd /tmp
pwd
cd ..
pwd
which ls
which not_existing_command
echo "After which test"
EOF

cd .. && ./mysh test/inputs/builtIn.txt > test/builtInOutput.txt 2>&1

if grep -q "/tmp" test/builtInOutput.txt && grep -q "/bin/ls" test/builtInOutput.txt && grep -q "After which test" test/builtInOutput.txt; then
    exit 0
else
    cat test/builtInOutput.txt
    exit 1
fi
