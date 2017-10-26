package(default_visibility = ["//visibility:public"])

exports_files([
    "node_configure.bzl",
    "kythe_pkg_config.bzl",
    "build_rules/verifier_test.sh.in",
])

# A binary that will cat or zcat arguments, based on file suffix.
sh_binary(
    name = "zcatext",
    srcs = ["zcatext.sh"],
)

# Script to extract bundled C++ indexer tests.
py_binary(
    name = "unbundle",
    srcs = ["unbundle.py"],
)
