load("@npm//@bazel/typescript:index.bzl", "ts_config", "ts_library")
load("@npm//@bazel/jasmine:index.bzl", "jasmine_node_test")
load("@build_bazel_rules_nodejs//:index.bzl", "nodejs_test")

package(default_visibility = ["//kythe:default_visibility"])

ts_config(
    name = "tsconfig",
    src = "tsconfig.json",
    deps = [],
)

ts_library(
    name = "kythe",
    srcs = ["kythe.ts"],
    tsconfig = ":tsconfig",
)

ts_library(
    name = "indexer",
    srcs = [
        "indexer.ts",
        "utf8.ts",
    ],
    tsconfig = ":tsconfig",
    deps = [
        ":kythe",
        "//kythe/proto:common_ts_proto",
        "@npm//@types/node",
        "@npm//google-protobuf",
        "@npm//typescript",
    ],
)

ts_library(
    name = "test_ts",
    srcs = ["test.ts"],
    tsconfig = ":tsconfig",
    deps = [
        ":indexer",
        ":kythe",
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

nodejs_test(
    name = "indexer_test",
    data = [":test_ts"] + glob(["testdata/**/*"]) + [
        "//kythe/go/platform/tools/entrystream",
        "//kythe/cxx/verifier",
    ],
    entry_point = ":test.ts",
)

jasmine_node_test(
    name = "utf8_test",
    deps = [":utf8_test_ts"],
)
