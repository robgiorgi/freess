#!/bin/bash

echo $0
testdate1="${0##*run-}"
testdate2="${testdate1%.sh}"
echo "TEST: $testdate2"

testprog="program-$testdate2"

cat <<EOF >$testprog
#! 1
#x 1 1000
#x 4 4000
#x 5 5000
#x 6 6000
1  3  4  2  # LW
1  7  5  2  # LW
7  7  7  3  # MUL
6  1  1  -1 # ADDI
2  7  6  2  # SW
6  2  2  8  # ADDI
4  1  0  -7 # BNE
EOF


cmd="./freess -exe $testprog -pw 4 -wins 16 -pregs 24 -robs 99 -lqs 3 -sqs 3 -llat 2 -afu 1 $*"
echo "$cmd"
$cmd
