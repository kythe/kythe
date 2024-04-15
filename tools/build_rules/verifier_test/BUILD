load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("@rules_python//python:defs.bzl", "py_binary")

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
        "@rules_proto//proto:defs",
    ],
)

bzl_library(
    name = "java_verifier_test_bzl",
    srcs = ["java_verifier_test.bzl"],
    deps = [
        ":verifier_test_bzl",
        "@rules_java//java:rules",
    ],
)

bzl_library(
    name = "verifier_test_bzl",
    srcs = ["verifier_test.bzl"],
    deps = [":verifier_test_impl_bzl"],
)

bzl_library(
    name = "verifier_test_impl_bzl",
    srcs = ["verifier_test_impl.bzl"],
    visibility = ["//visibility:private"],
    deps = [
        "@bazel_skylib//lib:shell",
    ],
)

bzl_library(
    name = "jvm_verifier_test_bzl",
    srcs = ["jvm_verifier_test.bzl"],
    deps = [
        ":verifier_test_bzl",
        "@rules_java//java:rules",
    ],
)

bzl_library(
    name = "kzip_archive_bzl",
    srcs = ["kzip_archive.bzl"],
)
