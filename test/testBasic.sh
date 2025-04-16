#!/bin/bash

echo "TESTING BASIC COMMANDS"

# Create input file
cat > inputs/basic.txt << EOF
echo Hello World
ls -la
sleep 1
echo Done sleeping
EOF

# Run mysh in batch mode
cd .. && ./mysh test/inputs/basic.txt > test/basicOutput.txt 2>&1

# Check
if grep -q "Hello World" test/basicOutput.txt && grep -q "Done sleeping" test/basicOutput.txt; then
    exit 0
else
    cat test/basicOutput.txt
    exit 1
fi
