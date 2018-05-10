load("//tools:build_rules/shims.bzl", "go_binary")

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
        "@org_golang_x_net//context:go_default_library",
    ],
)
