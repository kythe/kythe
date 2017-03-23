#!/bin/bash
# This script checks that the verifier properly handles singleton checking.
HAD_ERRORS=0
VERIFIER="../verifier"
TEST_INPUT="$1"
TEST_EXPECTED="$2"
cd "$(dirname "$0")"
"${VERIFIER}" --check_for_singletons=true \
    "${TEST_INPUT}" < /dev/null 2>&1 \
    | sed '/0x[0-9a-fA-F]*/d' \
    | diff - "${TEST_EXPECTED}"
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
