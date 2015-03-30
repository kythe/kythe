package(default_visibility = ["//visibility:public"])

licenses(["unencumbered"])  # Authored by Google, Apache 2.0

exports_files(["LICENSE"])

# Used by Bazel. If a test program depends on this target, it is
# assumed to be sharding-compliant.
exports_files(["test_sharding_compliant"])

filegroup(
    name = "srcs",
    srcs = glob(["**"]) + [
        "//tools/jdk:srcs",
        "//tools/genrule:srcs",
        "//tools/cpp:srcs",
        "//tools/test:srcs",
    ],
)
