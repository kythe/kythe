package(default_visibility = ["//visibility:public"])

load("//tools/build_rules/lexyacc:lexyacc.bzl", "lexyacc_toolchain", "lexyacc_variables")

exports_files(["lexyacc.bzl"])

toolchain_type(name = "toolchain_type")

lexyacc_variables(name = "current_lexyacc_toolchain")

lexyacc_toolchain(
    name = "lexyacc_remote",
    lex = "/usr/bin/flex",
    yacc = "/usr/bin/bison",
)
