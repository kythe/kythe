load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

package(default_visibility = ["//visibility:public"])

licenses(["unencumbered"])  # Authored by Google, Apache 2.0

exports_files(glob(["*"]))

filegroup(
    name = "grep-includes",
    srcs = ["dummy.sh"],
)

bzl_library(
    name = "toolchain_utils",
    srcs = ["toolchain_utils.bzl"],
)
