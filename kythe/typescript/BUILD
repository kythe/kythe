package(default_visibility = ["//kythe:default_visibility"])

load("@npm_bazel_typescript//:index.bzl", "ts_library", "ts_config")
load("@npm_bazel_jasmine//:index.bzl", "jasmine_node_test")

ts_config(
    name = "tsconfig",
    src = "tsconfig.json",
    deps = [],
)

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
        "@npm//source-map-support",
        "@npm//typescript",
    ],
)

ts_library(
    name = "utf8_test_ts",
    srcs = ["utf8_test.ts"],
    tsconfig = ":tsconfig",
    deps = [
        ":indexer",
        "@npm//@types/jasmine",
        "@npm//typescript",
    ],
)

jasmine_node_test(
    name = "utf8_test",
    deps = [":utf8_test_ts"],
)
