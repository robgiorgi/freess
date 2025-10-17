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
7  4  2  5   # MUL
1  6  1  400 # LW
8  6  4  6   # DIV
2  6  1  400 # SW
6  1  1  8   # ADDI
9  3  1  800 # SGTI
3  3  0  -7  # BEQ
EOF


cmd="./freess -exe $testprog -pw 8 -wins 99 -pregs 8 -robs 99 -lqs 3 -sqs 3 -llat 1 -afu 1 -dlat 15 -dpipe no -dfu 1 -mfu 1 -mlat 8 -mpipe no -stck -1 -fw 1 -wblat 1 -cw 1 -ww 99 -dw 1 -fw 1 -pw 99 -ioi no -ioc no -slat 0 -wins 30 -preg 30 -spec no -iw 10 $*"
echo "$cmd"
$cmd
