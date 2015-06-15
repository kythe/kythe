load("/tools/build_rules/go", "go_binary")

go_binary(
    name = "server",
    srcs = [
        "server.go",
    ],
    deps = [
        "//kythe/go/services/filetree",
        "//kythe/go/services/search",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/search",
        "//kythe/go/serving/xrefs",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/table",
        "//third_party/go:context",
    ],
)
