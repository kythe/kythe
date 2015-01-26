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

# This script regenerates the cxx_extractor_resources.inc file, which contains
# Clang version-specific header data that gets embedded in the extractor
# executable.
# Use: ./rebuild_resources.sh path/to/resource/dir > cxx_extractor_resources.inc
RESOURCE_DIR="$1"
: ${RESOURCE_DIR:?Missing resource directory.}
echo "static const struct FileToc kClangCompilerIncludes[] = {"
for resource in ${RESOURCE_DIR}/include/*
do
  echo "{\"$(basename "${resource}")\","
  echo "R\"filecontent($(< ${resource}))filecontent\""
  echo "},"
done
echo "{nullptr, nullptr}};"
