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
6  2  2  3   # ADDI
2  2  1  400 # SW
6  1  1  4   # ADDI
3  2  0  -5  # BEQ
EOF


cmd="./freess -exe $testprog -pw 8 -wins 10 -pregs 8 -robs 20 -lqs 2 -sqs 2 -llat 5 -afu 1 -dlat 15 -dpipe no -dfu 1 -mfu 1 -mlat 8 -mpipe no -stck -1 -fw 4 -wblat 1 -cw 1 -ww 1 -dw 4 -pw 4 -iw 1 -ioi no -ioc no -slat 1 -wins 30 -preg 30 -spec yes -swaits yes -tom yes -ars 2 -brs 2 -lsrs 2 -mrs 2 -drs 2 -loadpri no -iter 4 -rsrel yes $*"
echo "$cmd"
$cmd
