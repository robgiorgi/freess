#!/bin/bash

tests="c2050623 c2060705a c2060705b c2100120 c2130204a c2130204b c2141107a c2141107b c2141107r c2171031 c2191030 c2191030w c2201103 c2221028 c2231030 c2241029"
exe="freess"

# Dependency check
if [ ! -s "./$exe" ]; then
   echo "cannot find './$exe'"
   exit 2
fi


####################### ARGUMENT MANAGEMENT START
addargs=()        # array of -a arguments (preserves spaces/quoting)
test_arg=""       # optional single positional TEST

usage() {
  echo "Usage: $0 [-a ARG]... [--] [TEST]"
  echo "  -a ARG   Add an additional argument (repeatable)."
  echo "  TEST     Optional single test name; otherwise uses the default list."
}

# Manual, permissive parser: options can appear anywhere
while [ "$#" -gt 0 ]; do
  case "$1" in
    -a)
      # -a requires one argument (accept anything, even if it starts with '-')
      if [ -z "${2-}" ]; then
        echo "Error: -a requires an argument." >&2
        usage
        exit 2
      fi
      addargs+=("$2")
      shift 2
      ;;
    --)
      # end of options; remaining (if any) should be positional
      shift
      break
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    -*)
      echo "Unknown option: $1" >&2
      usage
      exit 2
      ;;
    *)
      # positional TEST (allow only one)
      if [ -n "$test_arg" ]; then
        echo "Error: only one TEST may be specified." >&2
        usage
        exit 2
      fi
      test_arg="$1"
      shift
      ;;
  esac
done

# Anything left after '--' (or after parsing) counts toward TEST
if [ "$#" -gt 0 ]; then
  if [ -n "$test_arg" ] || [ "$#" -gt 1 ]; then
    echo "Error: only one TEST may be specified." >&2
    usage
    exit 2
  fi
  test_arg="$1"
  shift
fi

# Apply TEST override if provided
if [ -n "$test_arg" ]; then
  tests="$test_arg"
fi

# Flat string like your original ALLARGS, plus safe array form
ALLARGS="${addargs[*]}"

# --- Example use (safe quoting) ---
echo "Tests: $tests"
echo "Extra args (${#addargs[@]}): ${addargs[*]}"
####################### ARGUMENT MANAGEMENT END

# Create a unique temp file, cleaned up automatically.
tmpfile="$(mktemp "/tmp/myprog.XXXXXX")" || {
   printf "mktemp failed\n" >&2
   exit 1
}
# Clean up on exit or signals
trap 'rm -f "$tmpfile" "program-$t"' EXIT INT TERM
out2="$tmpfile"
count1="0"
countt="0"
ec="0"

# Loop on tests
for t in $tests; do
   src="./test/run-$t.sh"
   if [ ! -s "$src" ]; then
      echo "File '$src' not found!"
      ec="1"; exit $ec
   else
      printf "* Testing %12s" $t
      countt=`expr $countt + 1`
      $src -s yes $* |./extractcycles.sh > $tmpfile
      if [ -s "$tmpfile" ]; then
         out1="./test/$t.cycles"
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
   if [ "$ec" != "0" ]; then 
   if [ "$count1" = "$countt" -a "$count1" != "0" ]; then
      echo "Goodbye."
   else
      echo "ERRORS"
   fi
fi
