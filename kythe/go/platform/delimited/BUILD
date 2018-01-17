load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "delimited",
    srcs = [
        "copy.go",
        ":delimited.go",
    ],
    deps = [
        "//kythe/go/util/dedup",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "delimited_test",
    size = "small",
    srcs = ["delimited_test.go"],
    library = "delimited",
    visibility = ["//visibility:private"],
)
