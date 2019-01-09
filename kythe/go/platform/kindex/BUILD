load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "kindex",
    srcs = ["kindex.go"],
    deprecation = "Please use //kythe/go/platform/kzip",
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/delimited",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/ptypes",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)

go_test(
    name = "kindex_test",
    size = "small",
    srcs = ["kindex_test.go"],
    library = "kindex",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@io_bazel_rules_go//proto/wkt:any_go_proto",
    ],
)
