load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "pq",
    srcs = ["pq.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/services/filetree",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/pq",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/stream",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/profile",
        "//kythe/go/util/schema",
        "//kythe/proto:storage_proto_go",
        "@go_x_net//:context",
    ],
)
