load("@build_bazel_rules_nodejs//:index.bzl", "nodejs_binary")

nodejs_binary(
    name = "commitlint",
    data = [
        "@npm//@commitlint/cli",
        "@npm//@commitlint/config-conventional",
    ],
    entry_point = "@npm//:node_modules/@commitlint/cli/lib/cli.js",
    tags = ["manual"],
    visibility = ["//visibility:public"],
)
