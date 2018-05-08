load("//tools:build_rules/shims.bzl", "go_test", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "ptypes",
    srcs = ["ptypes.go"],
    deps = [
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_golang_protobuf//ptypes:go_default_library",
        "@io_bazel_rules_go//proto/wkt:any_go_proto",
    ],
)

go_test(
    name = "ptypes_test",
    size = "small",
    srcs = ["ptypes_test.go"],
    library = "ptypes",
    visibility = ["//visibility:private"],
    deps = ["@io_bazel_rules_go//proto/wkt:any_go_proto"],
)
