#!/bin/bash
# This script checks that kindex_tool -assemble is inverted by
# kindex_tool -explode.
export CAMPFIRE_ROOT=`pwd`
export KINDEX_TOOL_BIN=${CAMPFIRE_ROOT}/campfire-out/bin/kythe/cxx/tools/kindex_tool
export TEST_DATA=${CAMPFIRE_ROOT}/kythe/cxx/tools/testdata
export TEST_TEMP=${CAMPFIRE_ROOT}/campfire-out/test/kythe/cxx/tools/testdata
set -e
mkdir -p ${TEST_TEMP}
${KINDEX_TOOL_BIN} -assemble ${TEST_TEMP}/test.kindex \
  ${TEST_DATA}/java.kindex_UNIT \
  ${TEST_DATA}/java.kindex_cf28b786fa21d0c45156e8011ac809afc454703fa03d767a5aeeed382f902795
${KINDEX_TOOL_BIN} -explode ${TEST_TEMP}/test.kindex
diff ${TEST_DATA}/java.kindex_UNIT ${TEST_TEMP}/test.kindex_UNIT
diff ${TEST_DATA}/java.kindex_cf28b786fa21d0c45156e8011ac809afc454703fa03d767a5aeeed382f902795 \
    ${TEST_TEMP}/test.kindex_cf28b786fa21d0c45156e8011ac809afc454703fa03d767a5aeeed382f902795
