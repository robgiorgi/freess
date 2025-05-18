#!/bin/bash

if [ "$1" = "" ]; then
   echo "ERROR: you must specify a test date or exercise (e.g., 070628 or ex1)"
   exit -1
fi
if [ ! -s "./run-$1.sh" ]; then
   echo "ERROR: cannot find file ./run-$1.sh"
   exit -1
fi

./run-$1.sh -int no > out-$1.txt
enscript -o out-$1.ps -r -f Courier6 out-$1.txt
ps2pdf out-$1.ps
rm -f out-$1.ps
echo "--> OUTPUT IN FILE: out-$1.pdf"
