#!/bin/bash -e

# Include our common argument processing logic
. "$(dirname "$0")/docker-common.sh"

function pull_image() {
  local image="$(ensure_tag "$1")"
  echo Pulling $image
  local repo="$(docker_url "$image")"

  if ! docker pull "$repo"; then
    echo "Failed pulling: $repo" >&2
    exit 1
  fi

  if ! docker tag "$repo" "$image"; then
    echo "Failed tagging: $image" >&2
    exit 1
  fi
}

for img in $@; do
  pull_image "$img"
done
