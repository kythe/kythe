load("@build_bazel_rules_nodejs//:defs.bzl", "nodejs_binary")

nodejs_binary(
    name = "commitlint",
    data = [
        "@npm//@commitlint/cli",
        "@npm//@commitlint/config-conventional",
    ],
    entry_point = "@npm//:node_modules/@commitlint/cli/lib/cli.js",
    install_source_map_support = False,
    tags = ["manual"],
    visibility = ["//visibility:public"],
)
