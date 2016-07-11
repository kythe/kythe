load("//tools:build_rules/go.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "http_server",
    srcs = ["http_server.go"],
    deps = [
        "//kythe/go/services/filetree",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/grpc",
        "//kythe/go/services/graphstore/proxy",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/xrefs",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/table",
        "//kythe/go/storage/xrefs",
        "//kythe/go/util/flagutil",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:xref_proto_go",
        "@go_grpc//:grpc",
        "@go_x_net//:context",
        "@go_x_net//:http2",
    ],
)
