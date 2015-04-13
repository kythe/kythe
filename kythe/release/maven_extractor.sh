#!/bin/bash -e

# Copyright 2014 Google Inc. All rights reserved.
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

# kindex compilation extractor for Maven builds. For exclusive use within the
# google/kythe Docker container. The assumed repository root is /repo and
# /compilations/java will be used as the .kindex file output directory.
#
# usage: maven_extractor.sh

export KYTHE_ROOT_DIRECTORY=/repo
cd "$KYTHE_ROOT_DIRECTORY"

export KYTHE_OUTPUT_DIRECTORY=/compilations/java
mkdir -p "$KYTHE_OUTPUT_DIRECTORY"

mvn_print() {
  mvn help:evaluate -Dexpression="$1" | grep -Ev '^(\[|Download)' | xargs echo -n
}
mvn help:evaluate </dev/null # ensure plugin is loaded

group="$(mvn_print project.groupId)"
artifact="$(mvn_print project.artifactId)"
export KYTHE_CORPUS="$group-$artifact"
if [[ "$KYTHE_CORPUS" = "-" ]]; then
  export KYTHE_CORPUS=maven
fi

echo "Extracting $KYTHE_CORPUS" >&2

CP="$(mktemp)"
trap 'rm -f "$CP"' EXIT

mvn process-sources dependency:build-classpath -Dmdep.outputFile="$CP"
if [[ ! -s "$CP" ]]; then
  # Empty Maven classpath; fix up for the javac @ argument below
  echo > "$CP"
fi

SRCS="$(find -name '*.java')"

java -jar /kythe/bin/javac_extractor_deploy.jar -cp "@$CP" $SRCS
