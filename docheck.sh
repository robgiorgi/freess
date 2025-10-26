#!/bin/bash

tests="c2050623 c2060705a c2060705b c2100120 c2130204a c2130204b c2141107a"
if [ "$1" != "" ]; then
   tests="$1"
fi

# Create a unique temp file, cleaned up automatically.
tmpfile="$(mktemp "/tmp/myprog.XXXXXX")" || {
   printf "mktemp failed\n" >&2
   exit 1
}
# Clean up on exit or signals
trap 'rm -f "$tmpfile"' EXIT INT TERM
out2="$tmpfile"
count1="0"
countt="0"

# Loop on tests
for t in $tests; do
   src="./run-$t.sh"
   if [ ! -s "$src" ]; then
      echo "File '$src' not found!"
   else
      echo -n "* Testing $t  "
      countt=`expr $countt + 1`
      ./run-$t.sh -s yes $* |extractcycles.sh > $tmpfile
      if [ -s "$tmpfile" ]; then
         out1="$t.cycles"
         outd=`diff $out1 $out2`
         if [ -z "$outd" ]; then
            echo " --> OK"
            count1=`expr $count1 + 1`
         else
            echo ": (diff OLD vs NEW)"
            echo "======================"
            echo "$outd"
            echo "======================"
         fi
      else
         echo
         echo "  ERROR"
      fi
   fi
done

echo "-----------------------"
echo "* Passed $count1/$countt tests."
if [ "$count1" = "$countt" ]; then
   echo "Goodbye."
else
   echo "ERRORS"
fi
