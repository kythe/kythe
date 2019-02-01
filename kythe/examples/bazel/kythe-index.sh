#!/bin/bash -e
#
# Copyright 2015 The Kythe Authors. All rights reserved.
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
                      [--bazel_root path] [--kythe_repo path] [--webui path]

Utility script to run the Kythe toolset over the Bazel repository.  This script can run each
component of Kythe separately with the --extract, --analyze, --postprocess, and --serve flags or all
together with the --all flag.

Paths to output artifacts are controlled with the --graphstore and --serving_table flags.  Their
defaults are $TMPDIR/gs.bazel and $TMPDIR/serving.bazel, respectively.

The --serving_addr flag controls the listening address of the Kythe http_server.
It's default is localhost:8888.  Any --webui path provided will be passed to the
http_server as its --public_resources flag.

The --bazel_root and --kythe_repo flags control which Bazel repository to index
(default: $PWD) and an optional override for the Kythe repository.
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

TMPDIR=${TMPDIR:-"/tmp"}

BAZEL_ARGS=(--define kythe_corpus=github.com/bazel/bazel)
BAZEL_ROOT="$PWD"

GRAPHSTORE="$TMPDIR"/gs.bazel
SERVING_TABLE="$TMPDIR"/serving.bazel
SERVING_ADDR=localhost:8888
WEBUI=()

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
    --bazel_root)
      BAZEL_ROOT="$(realpath -s "$2")"
      shift ;;
    --kythe_repo)
      BAZEL_ARGS=("${BAZEL_ARGS[@]}" --override_repository "io_kythe=$(realpath -s "$2")")
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
    --webui|-w)
      WEBUI=(--public_resources "$2")
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
KYTHE_BIN="$BAZEL_ROOT/bazel-bin/external/io_kythe"
KYTHE_PROTO_BIN="$BAZEL_ROOT/bazel-bin/external/io_kythe_lang_proto"

if ! grep -q 'Kythe extraction setup' WORKSPACE; then
  echo "ERROR: $BAZEL_ROOT is not a correctly configured Bazel repository" >&2
  echo "Please pass the --bazel_root flag after running setup-bazel-repo.sh on an existing Bazel repository." >&2
  echo "Example: setup-bazel-repo.sh $BAZEL_ROOT" >&2
  exit 1
fi

if [[ -n "$EXTRACT" ]]; then
  echo "Extracting Bazel compilations..."
  rm -rf bazel-out/*/extra_actions/external/io_kythe/
  bazel build "${BAZEL_ARGS[@]}" \
    --keep_going --output_groups=compilation_outputs \
    --experimental_extra_action_top_level_only \
    --experimental_action_listener=@io_kythe//kythe/extractors:extract_kzip_cxx \
    --experimental_action_listener=@io_kythe//kythe/extractors:extract_kzip_java \
    --experimental_action_listener=@io_kythe//kythe/extractors:extract_kzip_protobuf \
    //src/{main,test}/... || true
fi

analyze() {
  local lang="$1"
  shift 1
  local xa="io_kythe/kythe/build/extract_kzip_${lang}_extra_action"
  local dedup_stream="$KYTHE_BIN/kythe/go/platform/tools/dedup_stream/dedup_stream"
  find -L "$BAZEL_ROOT"/bazel-out/*/extra_actions/external/"$xa" -name '*.kzip' | \
    parallel -t -L1 "$@" | \
    "$dedup_stream" >>"$GRAPHSTORE"
}

if [[ -n "$ANALYZE" ]]; then
  echo "Analyzing compilations..."
  echo "GraphStore: $GRAPHSTORE"

  rm -rf "$GRAPHSTORE"

  bazel build "${BAZEL_ARGS[@]}" \
    @io_kythe//kythe/go/platform/tools/dedup_stream \
    @io_kythe//kythe/cxx/indexer/cxx:indexer \
    @io_kythe//kythe/java/com/google/devtools/kythe/analyzers/java:indexer \
    @io_kythe_lang_proto//kythe/cxx/indexer/proto:indexer
  analyze cxx "$KYTHE_BIN/kythe/cxx/indexer/cxx/indexer"
  analyze java "$KYTHE_BIN/kythe/java/com/google/devtools/kythe/analyzers/java/indexer"
  analyze protobuf "$KYTHE_PROTO_BIN/kythe/cxx/indexer/proto/indexer"
  cd "$ROOT"
fi

if [[ -n "$POSTPROCESS" ]]; then
  echo "Post-processing GraphStore..."
  echo "Serving table: $SERVING_TABLE"

  rm -rf "$SERVING_TABLE"

  bazel build -c opt "${BAZEL_ARGS[@]}" \
    @io_kythe//kythe/go/serving/tools/write_tables
  "$KYTHE_BIN"/kythe/go/serving/tools/write_tables/write_tables \
    --entries "$GRAPHSTORE" --out "$SERVING_TABLE" \
    --experimental_beam_pipeline --experimental_beam_columnar_data
fi

if [[ -n "$SERVE" ]]; then
  echo "Launching Kythe server"
  echo "Address: http://$SERVING_ADDR"

  bazel build "${BAZEL_ARGS[@]}" \
    @io_kythe//kythe/go/serving/tools/http_server
  "$KYTHE_BIN"/kythe/go/serving/tools/http_server/http_server \
    "${WEBUI[@]}" \
    --listen "$SERVING_ADDR" \
    --serving_table "$SERVING_TABLE"
fi
