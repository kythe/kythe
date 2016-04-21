load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
        "@go_x_net//:context",
        "@go_protobuf//:proto",
    ],
    deps = [
        "@go_uuid//:uuid",
        "@go_x_net//:context",
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/vfs",
        "//kythe/go/platform/vfs/zip",
    ],
)
