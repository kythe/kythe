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

# Initializes the Bazel workspace.  Uses the GO environmental variables to pick
# the Go tool and falls back onto "$(which go)".

cd "$(dirname "$0")"

if [[ $(uname) == 'Darwin' ]]; then
  LNOPTS="-sf"
  realpath() {
    python -c 'import os, sys; print os.path.realpath(sys.argv[2])' $@
  }
  readlink() {
    if [[ -L "$2" ]]; then
      python -c 'import os, sys; print os.readlink(sys.argv[2])' $@
    else
      echo $2
    fi
  }
else
  LNOPTS="-sTf"
fi

if [[ -z "$GO" ]]; then
  if [[ -z "$(which go)" ]]; then
    echo 'You need to have go installed to build Kythe.'
    echo 'Please see http://kythe.io/contributing for more information.'
    exit 1
  fi
  if ! GO="$(realpath -s "$(which go)")"; then
    echo 'ERROR: could not locate `go` binary on PATH' >&2
    exit 1
  fi
fi

echo "Using go found at $GO" >&2
ln ${LNOPTS} "$GO" tools/go/go

# This must be the same C++ compiler used to build the LLVM source.
if [[ -z "$CLANG" ]]; then
  if [[ -z "$(which clang)" ]]; then
    echo 'You need to have clang installed to build Kythe.'
    echo 'Note: Some vendors install clang with a versioned name'
    echo '(like /usr/bin/clang-3.5). You can set the CLANG environment'
    echo 'variable to specify the full path to yours.'
    echo 'Please see http://kythe.io/contributing for more information.'
    exit 1
  fi
  CLANG="$(realpath -s $(which clang))"
fi

echo "Using clang found at ${CLANG}" >&2

# The C++ compiler looks at some fixed but version-specific paths in the local
# filesystem for headers. We can't predict where these will be on users'
# machines because (among other reasons) they sometimes include the version
# number of the compiler being used. We can interrogate Clang (and gcc, which
# thankfully has a similar enough output format) for these paths.

# We use realpath -s as well as readlink -e to allow compilers to employ various
# methods for path canonicalization (and because Bazel may not always allow paths
# with relative arcs in its whitelist).

BUILTIN_INCLUDES=$(${CLANG} -E -x c++ - -v 2>&1 < /dev/null \
  | sed -n '/search starts here\:/,/End of search list/p' \
  | sed '/#include.*/d
/End of search list./d' \
  | while read -r INCLUDE_PATH ; do
  printf "%s" "  cxx_builtin_include_directory: \"$(realpath -s ${INCLUDE_PATH})\"__EOL__"
if [[ $(uname) != 'Darwin' ]]; then
  printf "%s" "  cxx_builtin_include_directory: \"$(readlink -e ${INCLUDE_PATH})\"__EOL__"
fi
done)

sed "s|ADD_CXX_COMPILER|${CLANG}|g" tools/cpp/osx_gcc_wrapper.sh.in \
    > tools/cpp/clang
chmod +x tools/cpp/clang
cp tools/cpp/clang tools/cpp/clang++

# This gets used in configure for LLVM, which doesn't use the same working
# directory as bazel.
ABS_WRAPPER_SCRIPT="$(realpath -s $(which tools/cpp/clang))"

sed "s|ADD_CXX_COMPILER|${CLANG}|g
s|ABS_WRAPPER_SCRIPT|${ABS_WRAPPER_SCRIPT}|g
s|ADD_CXX_BUILTIN_INCLUDE_DIRECTORIES|${BUILTIN_INCLUDES}|g" \
    tools/cpp/CROSSTOOL.in | \
sed 's|__EOL__|\
|g' > tools/cpp/CROSSTOOL
