load("//tools:build_rules/shims.bzl", "go_test", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "indexpack",
    srcs = ["indexpack.go"],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/vfs",
        "//kythe/go/platform/vfs/zip",
        "@go_uuid//:uuid",
    ],
)

go_test(
    name = "indexpack_test",
    size = "small",
    srcs = ["indexpack_test.go"],
    library = "indexpack",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
