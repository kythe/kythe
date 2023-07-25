load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "delimited",
    srcs = [
        "copy.go",
        "delimited.go",
    ],
    importpath = "kythe.io/kythe/go/platform/delimited",
    deps = ["@org_golang_google_protobuf//proto"],
)

go_test(
    name = "delimited_test",
    size = "small",
    srcs = ["delimited_test.go"],
    library = ":delimited",
    visibility = ["//visibility:private"],
)
