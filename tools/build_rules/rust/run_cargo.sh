#!/bin/bash -e

loc=$1
cargo_args=$2
if which cargo >/dev/null; then
   cd $loc
   cargo $cargo_args
else
   echo >&2 '\nERROR: No Rust Installation Found\n'
   exit 1
fi

