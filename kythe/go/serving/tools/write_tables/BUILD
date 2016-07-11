load("//tools:build_rules/go.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "write_tables",
    srcs = ["write_tables.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/grpc",
        "//kythe/go/services/graphstore/proxy",
        "//kythe/go/serving/pipeline",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/stream",
        "//kythe/go/util/datasize",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/profile",
        "//kythe/proto:storage_proto_go",
        "@go_x_net//:context",
    ],
)
