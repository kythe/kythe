load("@rules_python//python:defs.bzl", "py_binary")
load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

package(default_visibility = ["//visibility:public"])

exports_files([
    "verifier_test.sh.in",
    "indexer.sh.in",
])

py_binary(
    name = "unbundle",
    srcs = ["unbundle.py"],
    python_version = "PY3",
    srcs_version = "PY3",
)

bzl_library(
    name = "cc_indexer_test_bzl",
    srcs = ["cc_indexer_test.bzl"],
    deps = [
        ":verifier_test_bzl",
        "@bazel_skylib//lib:paths",
        "@bazel_skylib//lib:shell",
    ],
)

bzl_library(
    name = "java_verifier_test_bzl",
    srcs = ["java_verifier_test.bzl"],
    deps = [":verifier_test_bzl"],
)

bzl_library(
    name = "verifier_test_bzl",
    srcs = ["verifier_test.bzl"],
    deps = [
        "@bazel_skylib//lib:shell",
    ],
)

bzl_library(
    name = "jvm_verifier_test_bzl",
    srcs = ["jvm_verifier_test.bzl"],
    deps = [":verifier_test_bzl"],
)

bzl_library(
    name = "kzip_archive_bzl",
    srcs = ["kzip_archive.bzl"],
)

bzl_library(
    name = "rust_indexer_test_bzl",
    srcs = ["rust_indexer_test.bzl"],
    deps = [
        ":verifier_test_bzl",
        "@bazel_skylib//lib:paths",
    ],
)
