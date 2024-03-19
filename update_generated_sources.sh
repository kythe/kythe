#!/bin/bash -e
bazel build $(bazel query 'kind("_write_source_file", //...)')
