#!/bin/bash

if [ "$1" = "" ]; then
   echo "ERROR: you must specify a test date or exercise (e.g., 070628 or ex1)"
   exit -1
fi
if [ ! -s "./test/run-$1.sh" ]; then
   echo "ERROR: cannot find file ./test/run-$1.sh"
   exit -1
fi

./test/run-$1.sh -int no > out-$1.txt
#enscript -o out-$1.ps -r -f Courier6 out-$1.txt
a2ps -R out-$1.txt --columns 1 -o out-$1.ps -f 6
ps2pdf out-$1.ps
rm -f out-$1.ps
echo "--> OUTPUT IN FILE: out-$1.pdf"
