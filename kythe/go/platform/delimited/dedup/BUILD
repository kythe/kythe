load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "dedup",
    srcs = ["dedup.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/util/dedup",
        "@go_protobuf//:proto",
    ],
)
