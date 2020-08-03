package(default_visibility = ["//visibility:public"])

load("@rules_python//python:defs.bzl", "py_binary")
load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

exports_files(["verifier_test.sh.in"])

py_binary(
    name = "unbundle",
    srcs = ["unbundle.py"],
    python_version = "PY2",
)

bzl_library(
    name = "cc_indexer_test_bzl",
    srcs = ["cc_indexer_test.bzl"],
)

bzl_library(
    name = "java_verifier_test_bzl",
    srcs = ["java_verifier_test.bzl"],
)

bzl_library(
    name = "verifier_test_bzl",
    srcs = ["verifier_test.bzl"],
)

bzl_library(
    name = "jvm_verifier_test_bzl",
    srcs = ["jvm_verifier_test.bzl"],
)

bzl_library(
    name = "kzip_archive_bzl",
    srcs = ["kzip_archive.bzl"],
)

bzl_library(
    name = "rust_indexer_test_bzl",
    srcs = ["rust_indexer_test.bzl"],
)
