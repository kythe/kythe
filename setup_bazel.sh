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

# Initializes the Bazel workspace.  Uses the JAVA_HOME and GO environmental
# variables.  If either is unset, the paths will be determined by
# "$(which java)/.." and "$(which go)", respectively.

cd "$(dirname "$0")"

by_bin() {
  readlink -e "$(dirname "$(which "$1")")/.."
}

if [[ -z "$JAVA_HOME" ]]; then
  JAVA_HOME="$(by_bin java)"
fi

if [[ -z "$GO" ]]; then
  GO="$(readlink -e "$(which go)")"
fi

echo "Using jdk found in $JAVA_HOME" >&2
ln -sTf "$JAVA_HOME" tools/jdk/jdk

echo "Using go found at $GO" >&2
ln -sTf "$GO" tools/go/go

# This must be the same C++ compiler used to build the LLVM source.

CLANG="$(readlink -e $(which clang))"
echo "Using clang found at ${CLANG}"

# The C++ compiler looks at some fixed but version-specific paths in the local
# filesystem for headers. We can't predict where these will be on users'
# machines because (among other reasons) they sometimes include the version
# number of the compiler being used. We can interrogate Clang (and gcc, which
# thankfully has a similar enough output format) for these paths.

# We use realpath -s as well as readlink -e to allow compilers to employ various
# methods for path canonicalization (and because Bazel doesn't allow paths with
# relative arcs in its whitelist).

BUILTIN_INCLUDES=$(${CLANG} -E -x c++ - -v 2>&1 < /dev/null \
  | sed -n '/search starts here\:/,/End of search list/p' \
  | sed '/#include.*/d
/End of search list./d' \
  | while read -r INCLUDE_PATH ; do
  echo -n "  cxx_builtin_include_directory: \"$(realpath -s ${INCLUDE_PATH})\"\n"
  echo -n "  cxx_builtin_include_directory: \"$(readlink -e ${INCLUDE_PATH})\"\n"
done)

sed "s|ADD_CXX_COMPILER|${CLANG}|g
s|ADD_CXX_BUILTIN_INCLUDE_DIRECTORIES|${BUILTIN_INCLUDES}|g" \
    tools/cpp/CROSSTOOL.in \
    > tools/cpp/CROSSTOOL
