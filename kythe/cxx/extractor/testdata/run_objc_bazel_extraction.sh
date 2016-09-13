#!/bin/bash

if [ "$(uname)" != "Darwin" ]; then
  echo "Objective C Bazel extraction can only be done on darwin"
  exit 2
fi

BAZELOUT=$(bazel info workspace)/bazel-out
bazel build \
  --experimental_action_listener=//kythe/cxx/extractor:extract_kindex_objc \
  --experimental_extra_action_top_level_only \
  --experimental_proto_extra_actions \
  --ios_sdk_version=9.3 \
  :objc_lib

if [ $? -ne 0 ]; then
  echo "Build failed"
  exit 1
fi

# HACK: It is possible that there are other xa files in here.
D=$(pwd)
pushd $BAZELOUT
find . \
  -path "*/extra_actions/kythe/cxx/extractor/extra_action_objc/kythe/cxx/extractor/testdata/*.xa" \
  -exec cp {} "$D/objc_lib.xa" \; \
  && \
  chmod -x "$D/objc_lib.xa"
popd
