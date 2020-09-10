#!/bin/bash -e

# Copyright 2014 The Kythe Authors. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -o pipefail
export SHELL=/bin/bash

usage() {
  cat >&2 <<EOF
usage: kythe [--repo git-url] [--extract extractor] [--index]
             [--ignore-unhandled] [--files config-path] [--files-excludes re1,re2]

example: docker run --rm -t -v "$HOME/repo:/repo" -v "$HOME/gs:/graphstore" \
           google/kythe --extract maven --index --files --files-excludes '(^|/)\.,^third_party'

Extraction:
  If given an --extract type, the compilations in the mounted /repo VOLUME (or the given --repo
  which will copied to /repo) will be extracted to the /compilations VOLUME w/ subdirectories for
  each compilation's language (e.g. /compilations/java, /compilations/go).

  Supported Extractors: maven

Indexing:
  If given the --index flag, each compilation in /compilations will be sent to a corresponding
  language indexer and the outputs will be stored in a GraphStore in the /graphstore VOLUME.  If a
  compilation is without a corresponding language indexer, an error will be reported unless
  --ignore-unhandled is set.

  To emit file nodes for the entire repository, use the --files flag to specify a JSON file VNames
  configuration relative to the repository root.  --files-excludes can be used to exclude certain
  paths by a comma-separated list regex patterns.  It is highly recommended to exclude build
  output directories such as '(^|/)target'.  The --index flag is required for --files to be handled.

  Supported Languages: java,c++
EOF
}

usage_error() {
  echo "ERROR: $*" >&2
  usage
  exit 1
}

error() {
  echo "ERROR: $*" >&2
  exit 1
}

cleanup() {
  fix_permissions /repo
  fix_permissions /compilations
  fix_permissions /graphstore
  fix_permissions /root/.m2
}
trap cleanup EXIT

REPO=
IGNORE_UNHANDLED=
EXTRACTOR=
INDEXING=
FILES_CONFIG=
FILES_EXCLUDES='(^|/)\.'

while [[ $# -gt 0 ]]; do
  case "$1" in
    --repo|-r)
      REPO="$2"
      shift ;;
    --extract|-e)
      EXTRACTOR="$2"
      shift ;;
    --files|-f)
      FILES_CONFIG="$2"
      shift ;;
    --files-excludes)
      FILES_EXCLUDES="$2"
      shift ;;
    --index|-i)
      INDEXING=1 ;;
    --ignore-unhandled)
      IGNORE_UNHANDLED=1 ;;
    --help|-h)
      usage
      exit 0 ;;
    *) usage_error "Unknown argument: $1" ;;
  esac
  shift
done

mkdir -p /repo /compilations /graphstore

if [[ -n "$REPO" ]]; then
  if [ ! "$(ls -A /repo)" ]; then
    error '/repo not empty when given --repo'
  fi
  git clone "$REPO" /repo
fi

case "$EXTRACTOR" in
  maven)
    echo 'Extracting compilations' >&2
    "${EXTRACTOR}_extractor" ;;
  "")
    echo 'Skipping extraction' >&2 ;;
  *)
    error "Unknown extractor: '$EXTRACTOR'" ;;
esac

if [[ -z "$INDEXING" ]]; then
  echo 'Skipping indexing' >&2
  exit
fi

drive_indexer_kzip() {
  local lang="$(basename "$(dirname "$1")")"
  local analyzer="/kythe/bin/${lang}_indexer"
  if [[ ! -x "$analyzer" ]]; then
    if [[ -n "$IGNORE_UNHANDLED" ]]; then
      return 0
    else
      echo "Unhandled index file for '$lang': $*" >&2
      return 1
    fi
  fi
  echo "Indexing $*" >&2
  "$analyzer" "$@"
}
export -f drive_indexer_kzip
export IGNORE_UNHANDLED

find /compilations -name '*.kzip' | sort -R | \
  { parallel --gnu -L1 drive_indexer_kzip || echo "$? analysis failures" >&2; } | \
    dedup_stream | \
    write_entries --workers 12 --graphstore /graphstore

if [[ -z "$FILES_CONFIG" ]]; then
  echo "Skipping repository files indexing" >&2
  exit
fi

echo 'Emitting nodes for repository' >&2
cd /repo/ && \
  index_repository --vnames "$FILES_CONFIG" --exclude "$FILES_EXCLUDES" | \
  write_entries --workers 4 --graphstore /graphstore
