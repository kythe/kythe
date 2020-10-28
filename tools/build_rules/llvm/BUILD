load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

package(
    default_visibility = ["//visibility:public"],
)

exports_files([
    "configure_file.bzl",
    "generated_cmake_targets.bzl",
    "genearted_llvm_build_deps.bzl",
    "llvm.bzl",
])

bzl_library(name = "llvm_bzl")

bzl_library(
    name = "generated_llvm_build_targets_bzl",
    srcs = ["generated_llvm_build_targets.bzl"],
)
