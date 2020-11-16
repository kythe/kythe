load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

package(
    default_visibility = ["//visibility:public"],
)

exports_files([
    "configure_file.bzl",
    "generated_cmake_targets.bzl",
    "llvm.bzl",
])

bzl_library(name = "llvm_bzl")
