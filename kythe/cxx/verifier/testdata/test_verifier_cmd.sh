#!/bin/bash
VERIFIER=campfire-out/bin/kythe/cxx/verifier/verifier
${VERIFIER}
HAD_ERRORS=$?
if [ ${HAD_ERRORS} -ne 1 ]; then
  echo "[ FAIL: Verifier did not fail on lack of input script ]"
  exit 1
else
  echo "[ OK: Verifier failed on lack of input script ]"
fi
