#!/bin/bash
bazel run //third_party/bazel:update && bazel run //:gazelle -- "$(dirname "$0")"
