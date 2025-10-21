#!/usr/bin/env bash
# extract_F_to_C.sh — F..C columns (including negatives) for 000..last
# Usage: ./extract_F_to_C.sh < input.txt   OR   ./extract_F_to_C.sh dump.txt
set -o pipefail

input="/dev/stdin"
if [ $# -ge 1 ]; then
  if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    grep '^# ' "$0" | cut -c3-
    exit 0
  fi
  if [ ! -r "$1" ]; then
    echo "ERROR: cannot read input file: $1" >&2
    exit 1
  fi
  input="$1"
fi

awk '
BEGIN { OFS = "\t"; 
#   print "INS","F","D","P","I","X","W","C"
}

/^[0-9][0-9][0-9]\]/ {
  ins = substr($1, 1, 3)
  count = 0
  delete nums

  for (i = 1; i <= NF; i++) {
    # accept optional leading minus to keep negatives intact
    if ($i ~ /^-?[0-9]+$/) {
      count++
      nums[count] = $i
      if (count == 7) break
    }
  }

  if (count == 7) {
#    print ins, nums[1], nums[2], nums[3], nums[4], nums[5], nums[6], nums[7]
    print nums[3], nums[4], nums[5], nums[6], nums[7]
    seen = 1
  }
}

END {
  if (!seen) {
    print "WARNING: no instruction rows found in input." > "/dev/stderr"
  }
}
' "$input"

