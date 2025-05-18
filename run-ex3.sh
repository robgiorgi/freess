#!/bin/bash

echo $0
testdate1="${0##*run-}"
testdate2="${testdate1%.sh}"
echo "TEST DATE: $testdate2"

testprog="program-$testdate2"

cat <<EOF >$testprog
#! 1
1  2  1   0
6  2  2   1
2  2  1   0
6  1  1   4
4  2  0  -5
EOF

./freess -exe $testprog -wins 12 -robs 12 -pregs 12 -cw 1 -pw 2 -dw 2 -iw 2 -fw 2 -lqs 2 -sqs 2 -llat 1 -slat 1 -afu 1 $*
