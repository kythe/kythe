load("//tools:build_rules/shims.bzl", "go_binary")

go_binary(
    name = "export_sstable",
    srcs = ["export_sstable.go"],
    visibility = ["//visibility:public"],
    deps = [
        "//kythe/go/services/graphstore/proxy",
        "//kythe/go/storage/kvutil",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/pebble",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/log",
        "//kythe/go/util/profile",
        "@com_github_cockroachdb_pebble//objstorage/objstorageprovider",
        "@com_github_cockroachdb_pebble//sstable",
        "@com_github_cockroachdb_pebble//vfs",
    ],
)
