load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "dedup",
    srcs = ["dedup.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/util/dedup",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)
