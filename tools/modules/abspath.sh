#!/bin/bash
# Compatibility script for OS X which lacks realpath
if [ ! -z "$(which realpath)" ]; then
  realpath -s "$1"
else
  [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
fi
