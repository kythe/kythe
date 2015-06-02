#!/bin/bash
# This script runs the indexer on various test cases, piping the results
# to the verifier. The test cases contain assertions for the verifier to
# verify. Should every case succeed, this script returns zero.
HAD_ERRORS=0
BASE_DIR="$TEST_SRCDIR/kythe/cxx/indexer/cxx/testdata"
OUT_DIR="$TEST_TMPDIR"
VERIFIER="kythe/cxx/verifier/verifier"
INDEXER="kythe/cxx/indexer/cxx/indexer"
EXTRACTOR="kythe/cxx/extractor/cxx_extractor"

function one_case {
  local BUNDLE_FILE="${1}"
  local STANDARD="${2}"
  local VERIFIER_ARG="${3}"
  local BUNDLE_SHA=$(sha1sum "$1" | cut -f1 -d" ")
  local TEMP_PREFIX="${OUT_DIR}"/"${BUNDLE_SHA}"
  local BUNDLE_COPY="${TEMP_PREFIX}"/bundle.hcc
  rm -rf -- "${TEMP_PREFIX}"
  mkdir -p "${TEMP_PREFIX}"/test_bundle
  echo "#example test.cc" > "${BUNDLE_COPY}"
  cat "${BUNDLE_FILE}" >> "${BUNDLE_COPY}"
  # Split the bundle into files via "#example file.name" delimiter lines.
  pushd "${TEMP_PREFIX}" > /dev/null
  awk '/#example .*/{x="test_bundle/"$2;next}{print > x;}' bundle.hcc
  popd > /dev/null
  KYTHE_ROOT_DIRECTORY="$PWD" KYTHE_OUTPUT_DIRECTORY="${TEMP_PREFIX}" \
      KYTHE_VNAMES="${BASE_DIR}"/test_vnames.json "${EXTRACTOR}" \
      -c -std="${STANDARD}" "${TEMP_PREFIX}"/test_bundle/test.cc
  local KINDEX_FILE=$(find ${TEMP_PREFIX} -iname *.kindex)
  "${INDEXER}" -claim_unknown=false "${KINDEX_FILE}" \
      | "${VERIFIER}" ${VERIFIER_ARG} "${TEMP_PREFIX}"/test_bundle/*
  local RESULTS=( ${PIPESTATUS[0]} ${PIPESTATUS[1]} )
  if [ ${RESULTS[0]} -ne 0 ]; then
    echo "[ FAILED INDEX: $BUNDLE_FILE (${INDEXER} -claim_unknown=false ${KINDEX_FILE}) ]"
    HAD_ERRORS=1
  elif [ ${RESULTS[1]} -ne 0 ]; then
    echo "[ FAILED VERIFY: $BUNDLE_FILE ]"
    HAD_ERRORS=1
  else
    echo "[ OK: $BUNDLE_FILE ]"
  fi
}

# Remember to add these files to CAMPFIRE as well.
one_case "${BASE_DIR}/bundle_self_test.cc" "c++1y"
one_case "${BASE_DIR}/bundle_self_test_unclaimed.cc" "c++1y"
one_case "${BASE_DIR}/bundle_self_test_mix.cc" "c++1y"
one_case "${BASE_DIR}/bundle_self_test_multi_transcript.cc" "c++1y" "--ignore_dups"
one_case "${BASE_DIR}/bundle_self_test_vnames_json.cc" "c++1y"
one_case "${BASE_DIR}/claim_macro_features.cc" "c++1y"
exit ${HAD_ERRORS}
