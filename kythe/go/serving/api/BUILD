load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "@go_grpc//:grpc",
        "@go_x_net//:context",
        "//kythe/go/services/filetree",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/xrefs",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/table",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:xref_proto_go",
    ],
)
