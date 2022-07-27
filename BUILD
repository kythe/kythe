load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("//:version.bzl", "MAX_VERSION", "MIN_VERSION")
load("@bazel_gazelle//:def.bzl", "gazelle")
load("@rules_rust//proto:toolchain.bzl", "rust_proto_toolchain")

package(default_visibility = ["//visibility:private"])

exports_files(glob(["*"]))

filegroup(
    name = "nothing",
    visibility = ["//visibility:public"],
)

sh_test(
    name = "check_bazel_versions",
    srcs = ["//tools:check_bazel_versions.sh"],
    args = [
        MIN_VERSION,
        MAX_VERSION,
    ],
    data = [
        ".bazelminversion",
        ".bazelversion",
    ],
)

# gazelle:build_file_name BUILD
# gazelle:exclude kythe
# gazelle:exclude third_party
# gazelle:prefix kythe.io
gazelle(name = "gazelle")

bzl_library(
    name = "external_bzl",
    srcs = ["external.bzl"],
)

bzl_library(
    name = "version_bzl",
    srcs = ["version.bzl"],
)

bzl_library(
    name = "visibility_bzl",
    srcs = ["visibility.bzl"],
)

bzl_library(
    name = "setup_bzl",
    srcs = ["setup.bzl"],
)

# Create a Rust protobuf toolchain with protobuf version 2.8.2
rust_proto_toolchain(
    name = "rust_proto_toolchain_impl",
    edition = "2021",
    proto_compile_deps = ["@crate_index//:protobuf"],
    proto_plugin = "@crate_index//:protobuf-codegen__protoc-gen-rust",
)

toolchain(
    name = "rust_proto_toolchain",
    toolchain = ":rust_proto_toolchain_impl",
    toolchain_type = "@rules_rust//proto:toolchain",
)
