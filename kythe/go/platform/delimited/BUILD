load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "delimited",
    srcs = [
        "copy.go",
        ":delimited.go",
    ],
    deps = [
        "//kythe/go/util/dedup",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)

go_test(
    name = "delimited_test",
    size = "small",
    srcs = ["delimited_test.go"],
    library = "delimited",
    visibility = ["//visibility:private"],
)
