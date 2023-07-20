load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "triples",
    srcs = ["triples.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/proxy",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/stream",
        "//kythe/go/util/encoding/rdf",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:storage_go_proto",
    ],
)
