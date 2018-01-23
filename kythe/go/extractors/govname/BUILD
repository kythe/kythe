load("//tools:build_rules/testing.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "govname",
    srcs = ["govname.go"],
    deps = [
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/vnameutil",
        "//kythe/proto:storage_go_proto",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "govname_test",
    size = "small",
    srcs = ["govname_test.go"],
    library = "govname",
    visibility = ["//visibility:private"],
)
