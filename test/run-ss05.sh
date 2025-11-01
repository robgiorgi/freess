#!/bin/bash

echo $0
testdate1="${0##*run-}"
testdate2="${testdate1%.sh}"
echo "TEST: $testdate2"

testprog="program-$testdate2"

cat <<EOF >$testprog
#! 1
#x 1 10
#x 4 40
#x 5 50
#x 6 60
1  2  1  0   # LW
7  4  2  2   # MUL
2  4  1  0   # SW
6  1  1  4   # ADDI
4  2  0  -5  # BNE
EOF



cmd="./freess -exe $testprog -wins 8 -pregs 8 -robs 10 -llat 2 -lqs 3 -sqs 3 $*"
echo "$cmd"
$cmd
