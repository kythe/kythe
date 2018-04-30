#!/bin/bash

# Copyright 2018 Google Inc. All rights reserved.
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
#
# Reconfigures a pom.xml file to use the Kythe Java extractor and runs
# the build. Requires $JAVAC_WRAPPER to be set to the path of the Kythe
# javac-wrapper.sh script. Also requires the binary 'xmlstarlet' to be
# installed on accessible via $PATH.

set -e

JAVAC_WRAPPER=${JAVAC_WRAPPER:?"Error missing required env: JAVAC_WRAPPER"}
MAVEN_BUILD_FILE=${MAVEN_BUILD_FILE:="pom.xml"}
BUILD_TASK=${BUILD_TASK:="install"}

# Back up pom.xml file before modifying it.
cp $MAVEN_BUILD_FILE /tmp/pom.xml.backup
# Check if there is an existing maven-compiler-plugin.
COMPILER_PLUGIN_COUNT=`xmlstarlet sel -N x=http://maven.apache.org/POM/4.0.0 -t -v "count(/x:project/x:build/x:pluginManagement/x:plugins/x:plugin[x:artifactId='maven-compiler-plugin'])" $MAVEN_BUILD_FILE`

if (($COMPILER_PLUGIN_COUNT == 0))
then
  # Need to add the plugin explicitly; command-line options alone will not work.
  xmlstarlet ed -N x=http://maven.apache.org/POM/4.0.0 \
    -s "/x:project/x:build//x:pluginManagement/x:plugins" -t elem -n plugin-PLACEHOLDER -v "" \
    -s "//plugin-PLACEHOLDER" -t elem -n groupId -v "org.apache.maven.plugins" \
    -s "//plugin-PLACEHOLDER" -t elem -n artifactId -v "maven-compiler-plugin" \
    -s "//plugin-PLACEHOLDER" -t elem -n version -v "3.7" \
    -r "//plugin-PLACEHOLDER" -v "plugin" /tmp/pom.xml.backup \
  > $MAVEN_BUILD_FILE
fi

# Execute the build.
exec mvn clean $BUILD_TASK -Dmaven.compiler.forceJavacCompilerUse=true \
  -Dmaven.compiler.fork=true \
  -Dmaven.compiler.executable=$JAVAC_WRAPPER
# Restore the backup of the pom.xml file, removing our modifications.
mv /tmp/pom.xml.backup $MAVEN_BUILD_FILE
