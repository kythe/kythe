load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
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
