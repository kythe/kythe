package(default_visibility = ["//kythe:default_visibility"])

load("@npm_bazel_typescript//:index.bzl", "ts_library", "ts_config")

ts_library(
    name = "indexer",
    srcs = [
        "indexer.ts",
        "utf8.ts",
    ],
    tsconfig = ":tsconfig",
    deps = [
        "@npm//@types/node",
        "@npm//typescript",
    ],
)

ts_library(
    name = "test_ts",
    srcs = ["test.ts"],
    tsconfig = ":tsconfig",
    deps = [
        ":indexer",
        "@npm//@types/node",
        "@npm//typescript",
    ],
)

ts_config(
    name = "tsconfig",
    src = "tsconfig.json",
    deps = [],
)
