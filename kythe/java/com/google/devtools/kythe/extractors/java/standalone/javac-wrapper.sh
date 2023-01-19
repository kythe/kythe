#!/bin/bash

# Copyright 2015 The Kythe Authors. All rights reserved.
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
# repository is and where to put the resulting .kzip files, respectively.
#
# This script assumes a usable java binary is on $PATH or in $JAVA_HOME.
# Runtime options (e.g -Xbootclasspath/p:)can be passed to `java` with the
# $KYTHE_JAVA_RUNTIME_OPTIONS environment variable.
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

if [[ -z "$JAVA_HOME" ]]; then
  JAVABIN="$(command -v java)"
else
  JAVABIN="$JAVA_HOME/bin/java"
fi
readonly JAVABIN

if [[ -z "$REAL_JAVAC" ]]; then
  REAL_JAVAC="$(dirname "$(readlink -e "$0")")/javac.real"
fi
readonly REAL_JAVAC

if [[ -z "$JAVAC_EXTRACTOR_JAR" ]]; then
  JAVAC_EXTRACTOR_JAR="/opt/kythe/extractors/javac_extractor.jar"
fi
readonly JAVAC_EXTRACTOR_JAR

fix_permissions() {
  local dir="${1:?missing path}"
  # We cannot use stat -c to get user and group because it varies too much from
  # system to system.
  local ug
  # shellcheck disable=SC2012
  ug="$(ls -ld "$dir" | awk '{print $3":"$4}')"
  chown -R "$ug" "$dir"
}
cleanup() {
  fix_permissions "$KYTHE_ROOT_DIRECTORY"
  fix_permissions "$KYTHE_OUTPUT_DIRECTORY"
}
if [[ -n "$DOCKER_CLEANUP" ]]; then
  trap cleanup EXIT ERR INT
fi

# We do not want to inhibit word splitting here.
# shellcheck disable=SC2086
"$JAVABIN" \
  $KYTHE_JAVA_RUNTIME_OPTIONS \
  -jar "$JAVAC_EXTRACTOR_JAR" "$@" \
  1> >(sed -e 's/^/EXTRACT OUT: /') \
  2> >(sed -e 's/^/EXTRACT ERR: /' >&2)
if [[ -z "$KYTHE_EXTRACT_ONLY" ]]; then
  "$REAL_JAVAC" "$@"
fi
