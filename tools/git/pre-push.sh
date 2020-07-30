#!/bin/bash -e
#
# Copyright 2018 The Kythe Authors. All rights reserved.
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

# pre-push hook documentation: https://git-scm.com/docs/githooks#_pre_push

if [[ -n "$(git diff --name-only)" ]]; then
  echo "ERROR: found locally modified files; please push from a clean branch" >&2
  git diff --name-only >&2
  exit 1
elif [[ -n "$(git ls-files --others --exclude-standard )" ]]; then
  echo "ERROR: found local untracked files; please push from a clean branch" >&2
  git ls-files --others --exclude-standard >&2
  exit 1
fi

export WAIT_FOR_BAZEL=1
export USE_BAZELRC=1

# Ensure that we end back where we started
current_branch="$(git rev-parse --abbrev-ref HEAD)"
trap 'git checkout -q "$current_branch" || true' EXIT ERR INT

readonly z40=0000000000000000000000000000000000000000
remote="$1"

while read -r local_ref local_sha remote_ref remote_sha; do
  : "$remote_ref" "$remote_sha" # unused
  if [[ "$local_sha" == "$z40" ]]; then
    # Skip deletions
    continue
  fi

  branch="${local_ref#refs/heads/}"
  echo "Pre-push for branch: $branch" >&2
  if [[ $(git rev-parse --abbrev-ref HEAD) != "$branch" ]]; then
    git checkout -q "$branch"
  fi
  # TODO(#3173): remove need for arcanist
  if [[ "$KYTHE_SKIP_ARC_LINT" -ne 0 ]]; then
    arc lint --rev "$remote/master"
  fi
  arc unit --rev "$remote/master"
done
