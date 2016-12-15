#!/bin/bash

# Copyright 2015 Google Inc. All rights reserved.
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

# Wrapper for extracting compilation information from javac invocations.  It
# requires the same environment variables as the javac extractor (see
# AbstractJavacWrapper.java).  In particular, it needs KYTHE_ROOT_DIRECTORY and
# KYTHE_OUTPUT_DIRECTORY set to understand where the root of the compiled source
# repository is and where to put the resulting .kindex files, respectively.
#
# This script assumes a usable java binary is on $PATH.
#
# This script is meant as a replacement for $JAVA_HOME/bin/javac.  It assumes
# the true javac binary is in the same directory as itself and named
# "javac.real".  The location of the real javac binary can be configured with
# the REAL_JAVAC environment variable.  The default path for the javac extractor
# jar is /opt/kythe/javac_extractor.jar but can be set with the
# JAVAC_EXTRACTOR_JAR environment variable.
#
# If used in a Docker environment where KYTHE_ROOT_DIRECTORY and
# KYTHE_OUTPUT_DIRECTORY are volumes, it can be useful to set the DOCKER_CLEANUP
# environment variable so that files modified/created in either volume have
# their owner/group set to the volume's root directory's owner/group.
#
# Other environment variables that may be passed to this script include:
#   KYTHE_EXTRACT_ONLY: if set, suppress the call to javac after extraction
#   TMPDIR: override the location of extraction logs and other temporary output

export TMPDIR="${TMPDIR:-/tmp}"

if [[ -z "$REAL_JAVAC" ]]; then
  readonly REAL_JAVAC="$(dirname "$(readlink -e "$0")")/javac.real"
fi
if [[ -z "$JAVAC_EXTRACTOR_JAR" ]]; then
  readonly JAVAC_EXTRACTOR_JAR="/opt/kythe/extractors/javac_extractor.jar"
fi

fix_permissions() {
  local dir="${1:?missing path}"
  # We cannot use stat -c to get user and group because it varies too much from
  # system to system.
  local ug=$(ls -ld "$dir" | awk '{print $3":"$4}')
  chown -R "$ug" "$dir"
}
cleanup() {
  fix_permissions "$KYTHE_ROOT_DIRECTORY"
  fix_permissions "$KYTHE_OUTPUT_DIRECTORY"
}
if [[ -n "$DOCKER_CLEANUP" ]]; then
  trap cleanup EXIT ERR INT
fi

java -Xbootclasspath/p:"$JAVAC_EXTRACTOR_JAR" \
     -jar "$JAVAC_EXTRACTOR_JAR" \
     "$@" >>"$TMPDIR"/javac-extractor.out 2>> "$TMPDIR"/javac-extractor.err
if [[ -z "$KYTHE_EXTRACT_ONLY" ]]; then
  "$REAL_JAVAC" "$@"
fi
