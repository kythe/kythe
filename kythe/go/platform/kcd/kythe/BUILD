load("//tools:build_rules/testing.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "kythe",
    srcs = ["units.go"],
    deps = [
        "//kythe/go/platform/kcd",
        "//kythe/go/util/ptypes",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:buildinfo_go_proto",
        "//kythe/proto:storage_go_proto",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "kythe_test",
    size = "small",
    srcs = ["units_test.go"],
    library = "kythe",
    visibility = ["//visibility:private"],
    deps = ["//kythe/proto:any_go_proto"],
)
