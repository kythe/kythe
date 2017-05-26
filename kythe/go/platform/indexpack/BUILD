load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
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
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)
