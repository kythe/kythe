#!/bin/zsh -e
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

cd /tmp/jdk8

export KYTHE_CORPUS=openjdk
export KYTHE_ANALYSIS_TARGET=$(hg log -r "." --template "{latesttag}")
export KYTHE_ROOT_DIRECTORY=$PWD
export KYTHE_OUTPUT_DIRECTORY=/idx/
mkdir -p $KYTHE_OUTPUT_DIRECTORY

trap "fix_permissions $KYTHE_OUTPUT_DIRECTORY" EXIT
trap "fix_permissions $KYTHE_OUTPUT_DIRECTORY" ERR

cp=({build,jdk}/**/*.jar)
sp=(build/**/gensrc/ jdk/src/share/classes/)

find ${=sp} -name "*.java" > /tmp/srcs.txt

JAVAC_FLAGS=(
  -XDignore.symbol.file=true
  -Xprefer:source
  -implicit:none
  -cp ${(j/:/)cp}:${(j/:/)sp}
  @/tmp/srcs.txt)

echo "Extracting OpenJDK ($KYTHE_ANALYSIS_TARGET)" >&2
java -jar /kythe/bin/javac_extractor_deploy.jar $JAVAC_FLAGS
