#!/bin/bash

if "kythe/cxx/verifier/verifier"; then
  echo "[ FAIL: Verifier did not fail on lack of input script ]"
  exit 1
else
  echo "[ OK: Verifier failed on lack of input script ]"
fi
