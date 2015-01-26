#!/bin/bash -e
#
# Setup a watch notification on a GCS bucket for the xrefs module.
# See: https://cloud.google.com/storage/docs/object-change-notification#_Watching

GCS_PATH=gs://kythe-builds

PROJECT=kythe-repo
MODULE=xrefs
WATCHER_PATH=watcher
URL="https://${MODULE}-dot-${PROJECT}.appspot.com/${WATCHER_PATH}"

if ! gsutil notification watchbucket "$URL" "$GCS_PATH"; then
  echo 'You may need to authorize this notification: https://cloud.google.com/storage/docs/object-change-notification#_Authorization' >&2
  exit 1
fi
