load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("//tools/build_rules/lexyacc:lexyacc.bzl", "lexyacc_toolchain", "lexyacc_variables")

package(default_visibility = ["//visibility:public"])

exports_files(["lexyacc.bzl"])

toolchain_type(name = "toolchain_type")

lexyacc_variables(name = "current_lexyacc_toolchain")

lexyacc_toolchain(
    name = "lexyacc_remote",
    lex_path = "/usr/bin/flex",
    yacc_path = "/usr/bin/bison",
)

bzl_library(
    name = "lexyacc_bzl",
    srcs = ["lexyacc.bzl"],
)
