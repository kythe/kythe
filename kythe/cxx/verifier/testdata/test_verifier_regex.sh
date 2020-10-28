#!/bin/bash
# This script checks that the verifier properly applies goal regexes and
# computes the correct line and column information for diagnostics.
HAD_ERRORS=0
VERIFIER="../verifier"
cd "$(dirname "$0")"
"${VERIFIER}" --file_vnames=false --goal_regex='\s*\/\/\-\s*\[(.*)\]' \
    regex_input.txt < /dev/null 2>&1 \
    | diff - regex_expected_error.txt
RESULTS=( ${PIPESTATUS[0]} ${PIPESTATUS[2]} )
if [ ${RESULTS[0]} -ne 1 ]; then
  echo "[ VERIFIER DID NOT FAIL ]"
  HAD_ERRORS=1
elif [ ${RESULTS[1]} -ne 0 ]; then
  echo "[ WRONG ERROR TEXT ]"
  HAD_ERRORS=1
else
  echo "[ OK ]"
fi
exit ${HAD_ERRORS}
