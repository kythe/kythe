load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

package(
    default_visibility = ["//visibility:public"],
)

exports_files([
    "external_tools_configure.bzl",
    "external_tools_toolchain.bzl",
])

toolchain_type(name = "external_tools_toolchain_type")

bzl_library(name = "external_tools_toolchain_bzl")

bzl_library(
    name = "external_tools_configure_bzl",
    srcs = ["external_tools_configure.bzl"],
)
