#!/bin/bash -e

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

# Builds grpc-java, its dependencies, and places the resulting grpc-java library
# jar and java_plugin binary in third_party/grpc-java and Netty in
# third_party/netty.
#
# WARNING: installing the Netty dependency can take over an hour

THIRD_PARTY="$(readlink -e "$(dirname "$0")/..")"

TMP="$(mktemp -d)"
trap "rm -rf '$TMP'" EXIT ERR INT

git clone https://github.com/grpc/grpc-java.git "$TMP/grpc-java"
cd "$TMP/grpc-java"

git submodule update --init
pushd lib/netty
mvn install -pl codec-http2 -am -DskipTests=true
find */target/ -name '*-SNAPSHOT.jar' -exec cp '{}' "$THIRD_PARTY/netty/" \;
popd

git clone https://github.com/google/protobuf.git "$TMP/protobuf"

export PATH="$THIRD_PARTY/protobuf/bin:$PATH"
export CXXFLAGS="-I$TMP/protobuf/src/"
export LDFLAGS="-L$THIRD_PARTY/protobuf/lib/"
./gradlew install

rm -f "$THIRD_PARTY"/grpc-java/*.jar
cp -f all/build/libs/grpc-*-SNAPSHOT.jar "$THIRD_PARTY/grpc-java/"

rm -rf "$THIRD_PARTY/grpc-java/compiler"
cp compiler "$THIRD_PARTY/grpc-java/"
