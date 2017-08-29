load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "scan_leveldb",
    srcs = ["scan_leveldb.go"],
    deps = [
        "//kythe/go/storage/leveldb",
        "//kythe/go/util/flagutil",
        "//kythe/proto:serving_proto_go",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)
