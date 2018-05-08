load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "markedsource",
    srcs = ["markedsource.go"],
    deps = ["//kythe/proto:common_go_proto"],
)

go_test(
    name = "markedsource_test",
    size = "small",
    srcs = ["markedsource_test.go"],
    data = ["//kythe/cxx/doc"],
    library = "markedsource",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/proto:common_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
