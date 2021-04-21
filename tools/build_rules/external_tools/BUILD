load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load(":external_tools_toolchain.bzl", "external_tools_toolchain")

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

external_tools_toolchain(
    name = "external_tools_remote",
    asciidoc = "/usr/bin/asciidoc",
    path = "/bin:/usr/bin",
)

toolchain(
    name = "external_tools_remote_toolchain",
    toolchain = ":external_tools_remote",
    toolchain_type = ":external_tools_toolchain_type",
)
