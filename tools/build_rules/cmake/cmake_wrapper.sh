#!/bin/bash

realpath() {
  # OS X lacks the realpath binary, emulate it in shell.
  [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

invoke() {
  local _BUILDROOT="$1"
  local _ARGS=()

  while shift; [ $# -ne 0 ]; do
    case "$1" in
      "-D")
        shift
        _ARGS+=("-D", "$1")
        ;;
      "-C")
        shift
        _ARGS+=("-C", "$(realpath "$1")")
        ;;
      -*)
        echo "Unrecognized argument '$1'"
        exit 1
        ;;
      *)
        _ARGS+=("$(realpath "$1")")
        ;;
    esac
  done
  cd "$_BUILDROOT"
  touch "CMakeCache.txt"
  exec cmake "${_ARGS[@]}" > /dev/null
}

invoke "$@"
