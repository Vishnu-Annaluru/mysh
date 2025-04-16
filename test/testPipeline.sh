#!/bin/bash

echo "TESTING PIPELINE"

cat > inputs/pipeline.txt << EOF

echo line1 > pipelineTest.txt
echo line2 > tempLine2.txt
echo line3 > tempLine3.txt
cat tempLine2.txt > pipelineTest.txt
cat tempLine3.txt > pipelineTest.txt

cat pipelineTest.txt | grep line

echo line1 line2 line3 | grep line2

ls -la | grep test

rm pipelineTest.txt tempLine2.txt tempLine3.txt
EOF

cd .. && ./mysh test/inputs/pipeline.txt > test/pipelineOutput.txt 2>&1

if grep -q line test/pipelineOutput.txt && grep -q line2 test/pipelineOutput.txt; then
    exit 0
else
    cat test/pipelineOutput.txt
    exit 1
fi
