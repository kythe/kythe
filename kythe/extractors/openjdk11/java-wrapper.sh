#!/bin/bash -e
# Copyright 2019 The Kythe Authors. All rights reserved.
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
# Wraps the real java command; needed during JDK compilation to dispatch
# calls to the freshly compiled javac.
export KYTHE_ROOT_DIRECTORY="."
export KYTHE_OUTPUT_DIRECTORY="$HOME/jdkbuild/kythe/output"

JDK_VERSION=${JDK_VERSION:-11}  # The JDK version being built.
# Prefer JAVAC_EXTRACTOR_JAR, then Bazel runfiles.
if [[ -z $JAVAC_EXTRACTOR_JAR ]]; then
  JAR_PATH="io_kythe/kythe/java/com/google/devtools/kythe/extractors/java/standalone/javac9_extractor_deploy.jar"
  if [[ -f ${RUNFILES_DIR}/${JAR_PATH} ]]; then
    JAVAC_EXTRACTOR_JAR="${RUNFILES_DIR}/${JAR_PATH}"
  elif [[ -f ${RUNFILES_MANIFEST_FILE} ]]; then
    JAVAC_EXTRACTOR_JAR="$(grep -sm1 "^$JAR_PATH " "${RUNFILES_MANIFEST_FILE}" | cut -f2- -d' ')"
  elif [[ -f $0.runfiles/$JAR_PATH ]]; then
    JAVAC_EXTRACTOR_JAR="$0.runfiles/$JAR_PATH"
  else
    echo>&2 "ERROR: cannot find $JAR_PATH"
    exit 1
  fi
fi
if [[ ! -f $JAVAC_EXTRACTOR_JAR ]]; then
  echo>&2 "Unable to find path to java extractor jar"
  exit 1
fi
echo "Using extractor: $JAVAC_EXTRACTOR_JAR"

# Prefer JAVA_CMD, BOOT_JDK, JAVA_HOME, PATH.
if [[ -z $JAVA_CMD ]]; then
  if [[ $BOOT_JDK ]]; then
    JAVA_CMD="$BOOT_JDK/bin/java"
  elif [[ $JAVA_HOME ]]; then
    JAVA_CMD="$JAVA_HOME/bin/java"
  else
    JAVA_CMD="$(which java)"
  fi
fi
if [[ ! -x $JAVA_CMD ]]; then
  echo>&2 "Unable to find path to java binary"
  exit 1
fi
echo "Using java: $JAVA_CMD"

IS_BOOTSTRAP=0  # Whether this is a bootstrap compilation of the Java Compiler.
IS_JAVAC=0      # Whether this is an invocation of the Java Compiler.
declare -r ARGS=("$@")
declare -a EXTRACTOR_ARGS
while shift; [ $# -ne 0 ]; do
  case "$1" in
    -m | --module)
      if [[ $2 == *.javac.Main ]]; then
        JAVAC=1
        # Fails due to:
        #   java.lang.IllegalAccessException:
        #     unnamed module can't load com.sun.tools.javac.resources.compiler
        #     in module jdk.compiler.interim
        #EXTRACTOR_ARGS=(\
        #  "${EXTRACTOR_ARGS[@]}" \
        #  "-Xbootclasspath/a:$JAVAC_EXTRACTOR_JAR" \
        #  "com.google.devtools.kythe.extractors.java.standalone.Javac9Wrapper" \
        #  "-Xprefer:source" \
        #)

        # Fails due to:
        #   NPE in depend plugin
        #EXTRACTOR_ARGS=(\
        #  "${EXTRACTOR_ARGS[@]}" \
        #  --patch-module "jdk.compiler.interim=$JAVAC_EXTRACTOR_JAR" \
        #  --add-reads "jdk.compiler.interim=java.logging" \
        #  -m jdk.compiler.interim/com.google.devtools.kythe.extractors.java.standalone.Javac9Wrapper \
        #  -Xprefer:source \
        #)

        # Fails due to:
        #  not in a module on the module source path
        #EXTRACTOR_ARGS=( \
        #  "-jar" "$JAVAC_EXTRACTOR_JAR" \
        #  "-Xprefer:source" \
        #)

        # Fails because it can't find the system compiler, because the interim
        # compiler is jdk.compiler.interim. Hard-coding this path fails with
        # not in a module on the module source path.
        EXTRACTOR_ARGS=( \
          "${EXTRACTOR_ARGS[@]}" \
          "--add-exports=jdk.compiler.interim/com.sun.tools.javac.main=ALL-UNNAMED" \
          "--add-exports=jdk.compiler.interim/com.sun.tools.javac.util=ALL-UNNAMED" \
          "--add-exports=jdk.compiler.interim/com.sun.tools.javac.file=ALL-UNNAMED" \
          "--add-exports=jdk.compiler.interim/com.sun.tools.javac.api=ALL-UNNAMED" \
          "--add-exports=jdk.compiler.interim/com.sun.tools.javac.code=ALL-UNNAMED" \
          "-jar" "$JAVAC_EXTRACTOR_JAR" \
          "-Xprefer:source" \
        )
      else
        EXTRACTOR_ARGS=("${EXTRACTOR_ARGS[@]}" "$1" "$2")
      fi
      shift
      ;;
    -Xplugin:depend* | -Xlint:*)
      ;;
    --doclint-format)
      shift
      ;;
    -Xmx*)
      EXTRACTOR_ARGS=("${EXTRACTOR_ARGS[@]}" "-Xmx3G")
      ;;
    -target)
      if [[ $2 != "$JDK_VERSION" ]]; then
        BOOTSTRAP=1
      fi
      EXTRACTOR_ARGS=("${EXTRACTOR_ARGS[@]}" "$1" "$2")
      shift
      ;;
    --add-modules | --limit-modules)
      EXTRACTOR_ARGS=("${EXTRACTOR_ARGS[@]}" "$1" "$2,java.logging,java.sql")
      shift
      ;;
    *)
      EXTRACTOR_ARGS=("${EXTRACTOR_ARGS[@]}" "$1")
      ;;
  esac
done
if (($IS_JAVAC && !$IS_BOOTSTRAP)); then
  "$JAVA_CMD" "${EXTRACTOR_ARGS[@]}" 1>&2 || true
fi
exec "$JAVA_CMD" "${ARGS[@]}"
