load("//tools:build_rules/shims.bzl", "go_test", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "indexpack",
    srcs = ["indexpack.go"],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/vfs",
        "//kythe/go/platform/vfs/zip",
        "@com_github_golang_protobuf//jsonpb:go_default_library",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_pborman_uuid//:go_default_library",
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
    ],
)
