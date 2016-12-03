#!/bin/bash
# This script checks that the verifier properly applies goal regexes and
# computes the correct line and column information for diagnostics.
HAD_ERRORS=0
VERIFIER="kythe/cxx/verifier/verifier"
"${VERIFIER}" --goal_regex='\s*\/\/\-\s*\[(.*)\]' \
    "kythe/cxx/verifier/testdata/regex_input.txt"< /dev/null 2>&1 \
    | diff - kythe/cxx/verifier/testdata/regex_expected_error.txt
RESULTS=( ${PIPESTATUS[0]} ${PIPESTATUS[1]} )
if [ ${RESULTS[0]} -ne 1 ]; then
  echo "[ VERIFIER DID NOT FAIL: $1 ]"
  HAD_ERRORS=1
elif [ ${RESULTS[1]} -ne 0 ]; then
  echo "[ WRONG ERROR TEXT: $1 ]"
  HAD_ERRORS=1
else
  echo "[ OK: $1 ]"
fi
exit ${HAD_ERRORS}
