load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "table",
    srcs = ["table.go"],
    deps = [
        "//kythe/go/storage/keyvalue",
        "@go_protobuf//:proto",
    ],
)
