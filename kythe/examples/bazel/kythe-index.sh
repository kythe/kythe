#!/bin/bash -e
#
# Copyright 2015 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Utility script to run the Kythe toolset over the Bazel repository.  See the usage() function below
# for more details.

usage() {
  cat <<EOF
Usage: kythe-index.sh (--all | [--extract] [--analyze] [--postprocess] [--serve])
                      [--graphstore path] [--serving_table path] [--serving_addr host:port]
                      [--bazel-root path] [--kythe-release path]

Utility script to run the Kythe toolset over the Bazel repository.  This script can run each
component of Kythe separately with the --extract, --analyze, --postprocess, and --serve flags or all
together with the --all flag.

Paths to output artifacts are controlled with the --graphstore and --serving_table flags.  Their
defaults are /tmp/gs.bazel and /tmp/serving.bazel, respectively.

The --serving_addr flag controls the listening address of the Kythe http_server.  It's default is
localhost:8888.

The --bazel-root and --kythe-release flags control which Bazel repository to index and at what path
are the Kythe tools.  They default to $PWD and /opt/kythe, respectively.
EOF
}

if [[ $# -eq 0 ]]; then
  usage
  exit 1
fi

if ! command -v bazel >/dev/null; then
  echo "ERROR: bazel not found on your PATH" >&2
  exit 1
fi

KYTHE_RELEASE=/opt/kythe
BAZEL_ROOT="$PWD"

GRAPHSTORE=/tmp/gs.bazel
SERVING_TABLE=/tmp/serving.bazel
SERVING_ADDR=localhost:8888

EXTRACT=
ANALYZE=
POSTPROCESS=
SERVE=

while [[ $# -gt 0 ]]; do
  case "$1" in
    --help|-h)
      usage
      exit ;;
    --all|-a)
      EXTRACT=1
      ANALYZE=1
      POSTPROCESS=1
      SERVE=1 ;;
    --bazel-root)
      BAZEL_ROOT="$(realpath -s "$2")"
      shift ;;
    --kythe-release)
      KYTHE_RELEASE="$(realpath -s "$2")"
      shift ;;
    --graphstore|-g)
      GRAPHSTORE="$2"
      shift ;;
    --serving_table|-s)
      SERVING_TABLE="$(realpath -s "$2")"
      shift ;;
    --serving_addr|-l)
      SERVING_ADDR="$2"
      shift ;;
    --extract)
      EXTRACT=1 ;;
    --analyze)
      ANALYZE=1 ;;
    --postprocess)
      POSTPROCESS=1 ;;
    --serve)
      SERVE=1 ;;
    *)
      echo "ERROR: unknown argument: $1" >&2
      exit 1 ;;
  esac
  shift
done

cd "$BAZEL_ROOT"

if [[ ! -f third_party/kythe/BUILD ]]; then
  echo "ERROR: $BAZEL_ROOT is not a correctly configured Bazel repository" >&2
  echo "Please pass the --bazel-root flag after running setup-bazel-repo.sh on an existing Bazel repository." >&2
  echo "Example: setup-bazel-repo.sh $BAZEL_ROOT" >&2
  exit 1
fi

if [[ -n "$EXTRACT" ]]; then
  echo "Extracting Bazel compilations..."
  rm -rf bazel-out/*/extra_actions/third_party/kythe/
  set +e
  bazel build \
    --experimental_action_listener=//third_party/kythe:java_extract_kindex \
    --experimental_action_listener=//third_party/kythe:cxx_extract_kindex \
    //src/{main,test}/...
  set -e
fi

if [[ -n "$ANALYZE" ]]; then
  echo "Analyzing compilations..."
  echo "GraphStore: $GRAPHSTORE"

  rm -rf "$GRAPHSTORE"

  cd /tmp
  find -L "$BAZEL_ROOT"/bazel-out/*/extra_actions/third_party/kythe/java_extra_action -name '*.kindex' | \
    parallel -L1 "$KYTHE_RELEASE"/indexers/java_indexer.jar | \
    "$KYTHE_RELEASE"/tools/dedup_stream | \
    "$KYTHE_RELEASE"/tools/write_entries --graphstore "$GRAPHSTORE"
  cd "$ROOT"
fi

if [[ -n "$POSTPROCESS" ]]; then
  echo "Post-processing GraphStore..."
  echo "Serving table: $SERVING_TABLE"

  rm -rf "$SERVING_TABLE"

  "$KYTHE_RELEASE"/tools/write_tables --graphstore "$GRAPHSTORE" --out "$SERVING_TABLE"
fi

if [[ -n "$SERVE" ]]; then
  echo "Launching Kythe server"
  echo "Address: http://$SERVING_ADDR"

  "$KYTHE_RELEASE"/tools/http_server \
    --listen "$SERVING_ADDR" \
    --public_resources "$KYTHE_RELEASE"/web/ui \
    --serving_table "$SERVING_TABLE"
fi
