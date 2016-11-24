#!/bin/bash
set -o pipefail
set -o errexit
readonly WRITE_ENTRIES=kythe/go/storage/tools/write_entries/write_entries
readonly WRITE_TABLES=kythe/go/serving/tools/write_tables/write_tables
readonly KYTHE=kythe/go/serving/tools/kythe/kythe
readonly INDEXER=kythe/cxx/indexer/cxx/indexer
readonly INPUT_PATH="kythe/cxx/tools/testdata/def_decl_test.cc"
readonly INPUT_FILE="${TEST_SRCDIR}/io_kythe/${INPUT_PATH}"
readonly FILE_URI="kythe:?path=${INPUT_PATH}"
readonly OUTDIR="${TEST_TMPDIR:?Must be run via bazel test}"

"$INDEXER" -i "$INPUT_FILE" | "$WRITE_ENTRIES" --graphstore "${OUTDIR}/graph" > /dev/null 2>&1
"$WRITE_TABLES" --graphstore "${OUTDIR}/graph" --out "${OUTDIR}/serving" > /dev/null 2>&1

kythe() {
  "$KYTHE" -api "${OUTDIR}/serving" "$@"
}

for ticket in $(kythe decor "${FILE_URI}" | grep /kythe/edge/defines/binding | awk '{print $4}'); do
  XREFS=$(kythe xrefs -declarations all -definitions all "$ticket")
  if [ "$(echo "$XREFS" | grep "${INPUT_PATH}" | wc -l)" -lt 3 ]; then
    echo -e "Definitions and declarations not completely linked:\n$XREFS" 1>&2
    exit 1
  fi
done
echo "PASS"
