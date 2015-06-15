load("/tools/build_rules/go", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "entrystream",
    srcs = [
        "entrystream/entrystream.go",
    ],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/services/graphstore/compare",
        "//kythe/go/storage/stream",
        "//kythe/go/util/flagutil",
        "//kythe/proto:storage_proto_go",
        "//third_party/go:protobuf",
    ],
)

go_binary(
    name = "ktags",
    srcs = [
        "ktags/ktags.go",
    ],
    deps = [
        "//kythe/go/services/search",
        "//kythe/go/services/xrefs",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
        "//third_party/go:context",
    ],
)

go_binary(
    name = "dedup_stream",
    srcs = [
        "dedup_stream/dedup_stream.go",
    ],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/util/flagutil",
    ],
)

go_binary(
    name = "viewindex",
    srcs = [
        "viewindex/viewindex.go",
    ],
    deps = [
        "//kythe/go/platform/kindex",
        "//kythe/go/util/flagutil",
        "//third_party/go:context",
    ],
)

go_binary(
    name = "indexpack",
    srcs = [
        "indexpack/indexpack.go",
    ],
    deps = [
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/platform/vfs",
        "//kythe/go/platform/vfs/gcs",
        "//kythe/go/util/oauth2",
        "//kythe/proto:analysis_proto_go",
        "//third_party/go:cloud_storage",
        "//third_party/go:context",
    ],
)

sh_test(
    name = "indexpack_test",
    srcs = ["indexpack_test.sh"],
    data = [
        ":indexpack",
        ":viewindex",
        "//kythe/testdata:test.kindex",
        "//third_party/jq",
    ],
)
