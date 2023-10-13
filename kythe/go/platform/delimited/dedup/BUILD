load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "dedup",
    srcs = ["dedup.go"],
    importpath = "kythe.io/kythe/go/platform/delimited/dedup",
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/util/dedup",
        "@org_golang_google_protobuf//proto",
    ],
)
