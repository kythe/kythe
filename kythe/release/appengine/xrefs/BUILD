load("//tools:build_rules/go.bzl", "go_binary")

go_binary(
    name = "server",
    srcs = [
        "server.go",
    ],
    deps = [
        "@go_x_net//:context",
        "//kythe/go/services/filetree",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/xrefs",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/table",
    ],
)
