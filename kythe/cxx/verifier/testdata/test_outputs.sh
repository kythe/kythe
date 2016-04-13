#!/bin/bash
# This script checks that the verifier produces the correct debug
# output given pre-baked indexer output. Each test case should have
# a .bin (the result of redirecting the indexer's output to a file) and
# a .dot (the correct result from running the verifier using that
# output with the --graphviz flag). If the output matches exactly for
# each test case, this script returns zero.
HAD_ERRORS=0
BASE_DIR="$PWD/kythe/cxx/verifier/testdata"
VERIFIER="kythe/cxx/verifier/verifier"
function one_case {
  cat $1.bin | ${VERIFIER} --graphviz | diff $1.dot -
  DOT_RESULTS=( ${PIPESTATUS[1]} ${PIPESTATUS[2]} )
  if [ ${DOT_RESULTS[0]} -ne 0 ]; then
    echo "[ FAILED VERIFY --GRAPHVIZ: $1 ]"
    HAD_ERRORS=1
  elif [ ${DOT_RESULTS[1]} -ne 0 ]; then
    echo "[ WRONG VERIFY --GRAPHVIZ: $1 ]"
    HAD_ERRORS=1
  else
    echo "[ OK: $1 ]"
  fi
}

one_case "${BASE_DIR}/just_file_node"

exit ${HAD_ERRORS}
