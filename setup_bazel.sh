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

# Initializes the Bazel workspace.  Uses the GOROOT environmental variable to
# pick the Go tool and falls back onto "$(go env GOROOT)".

# The current version of Bazel we support is
# 53f407608ef6547bd690fa4420418aa6b6991b22

cd "$(dirname "$0")"

if [[ $(uname) == 'Darwin' ]]; then
  LNOPTS="-fsh"
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
  : ${OPENSSL_HOME:=/usr/local/opt/openssl}
  : ${UUID_HOME:=/usr/local/opt/ossp-uuid}
  : ${MEMCACHED_HOME:=/usr/local/opt/libmemcached}
  if [[ -d "${OPENSSL_HOME}/include" ]]; then
    ln -fsh "${OPENSSL_HOME}" third_party/openssl
  else
    echo 'Could not find OpenSSL.'
    echo 'Set the OPENSSL_HOME variable and try again.'
    exit 1
  fi
  if [[ -d "${UUID_HOME}/include" ]]; then
    ln "${LNOPTS}" "${UUID_HOME}" third_party/ossp-uuid
  else
    echo 'Could not find ossp-uuid.'
    echo 'Set the UUID_HOME variable and try again.'
    exit 1
  fi
  if [[ -d "${MEMCACHED_HOME}/include" ]]; then
    ln -fsh "${MEMCACHED_HOME}" third_party/libmemcached
  else
    echo 'Could not find libmemcached.'
    echo 'Set the MEMCACHED_HOME variable and try again.'
    exit 1
  fi
else
  LNOPTS="-sTf"
fi

if [[ -z "${NODEJS}" ]]; then
  if [[ -z "$(which node)" ]]; then
    echo 'No node.js installation found.'
    ln "${LNOPTS}" "$(which false)" tools/node
  else
    NODEJS="$(realpath -s $(which node))"
  fi
fi

if [[ ! -z "${NODEJS}" ]]; then
  echo "Using node.js found at ${NODEJS}" >&2
  ln "${LNOPTS}" "${NODEJS}" tools/node
fi
