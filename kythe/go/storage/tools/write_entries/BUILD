load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "write_entries",
    srcs = ["write_entries.go"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/proxy",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/stream",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/log",
        "//kythe/go/util/profile",
        "//kythe/proto:storage_go_proto",
    ],
)
