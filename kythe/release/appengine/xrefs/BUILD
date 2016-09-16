load("@io_bazel_rules_go//go:def.bzl", "go_binary")

go_binary(
    name = "server",
    srcs = [
        "server.go",
    ],
    deps = [
        "//kythe/go/services/filetree",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/xrefs",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/table",
        "@go_x_net//:context",
    ],
)
