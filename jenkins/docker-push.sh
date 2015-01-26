#!/bin/bash -e

# Include our common argument processing logic
. "$(dirname "$0")/docker-common.sh"

function push_image() {
  local image="$(ensure_tag "$1")"
  echo Pushing $image
  local repo="$(docker_url "$image")"

  if ! docker tag "$image" "$repo"; then
    echo "Failed tagging: $image" >&2
    exit 1
  fi

  if ! docker push "$repo"; then
    echo "Failed pushing: $repo" >&2
    exit 1
  fi
}

for img in $@; do
  push_image "$img"
done
