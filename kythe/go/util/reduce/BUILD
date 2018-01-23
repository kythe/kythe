load("//tools:build_rules/testing.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "reduce",
    srcs = [
        "reduce.go",
        ":reduce_sort.go",
    ],
    deps = [
        "//kythe/go/util/disksort",
        "//kythe/proto:internal_go_proto",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "reduce_test",
    size = "small",
    srcs = ["reduce_test.go"],
    library = "reduce",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
