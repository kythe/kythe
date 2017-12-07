load("//tools:build_rules/go.bzl", "go_release_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_release_binary(
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
        "//kythe/go/util/schema/edges",
        "//kythe/proto:storage_proto_go",
    ],
)
