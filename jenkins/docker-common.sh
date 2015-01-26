#!/bin/bash -e

DOCKER_NAMESPACE=gcr.io/kythe_repo
DEFAULT_TAG=latest
AUTH=1

while [[ $# -gt 0 ]]; do
  case "$1" in
    -n|--namespace)  DOCKER_NAMESPACE=$2;      shift 2;;
    -t|--tag)        DEFAULT_TAG=$2;           shift 2;;
    --no-auth)       AUTH=;                    shift 1;;
    --) shift; break ;;
    --*) echo "Unknown argument: $1" >&2; exit 1 ;;
    *) break ;;
  esac
done

if [[ -n "$AUTH" ]]; then
  gcloud preview docker --authorize_only
fi

ensure_tag() {
  local image="$1"
  if [[ "$image" != *:* ]]; then
    image="${image}:${DEFAULT_TAG}"
  fi
  echo "$image"
}

remove_repo_name() {
  sed -r 's#(.*/)?(.*)#\2#' <<<"$1"
}

docker_url() {
  echo "${DOCKER_NAMESPACE}/$(remove_repo_name "$1")"
}
