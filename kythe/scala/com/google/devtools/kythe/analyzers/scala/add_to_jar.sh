#!/bin/sh
# Add scalac-plugin.xml to the root of the jar.
# This is the struture required for scala compiler plugins
set -e
finalJar=${1}
shift
JAR_CMD="$(pwd)/external/local_jdk/bin/jar"

tmpDir="${finalJar}.tmp"
rm -rf "${tmpDir}"
mkdir "${tmpDir}"
# put xml file and all jars in tmp dir
cp "$@" "${tmpDir}/"
# unjar the deploy jar and delete all jars
(cd "${tmpDir}"
 ${JAR_CMD} xf ./*deploy.jar
 rm ./*.jar)

# rejar with xml file in right place
${JAR_CMD} cf "${finalJar}" -C "${tmpDir}" .
