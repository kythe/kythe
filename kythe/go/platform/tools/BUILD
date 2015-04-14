load("//tools/build_rules/go", "go_binary")

package(default_visibility = ["//visibility:public"])

go_binary(
    name = "entrystream",
    srcs = [
        "entrystream/entrystream.go",
    ],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/services/graphstore/compare",
        "//kythe/go/storage/stream",
        "//kythe/proto:storage_proto",
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
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:storage_proto",
        "//kythe/proto:xref_proto",
    ],
)

go_binary(
    name = "dedup_stream",
    srcs = [
        "dedup_stream/dedup_stream.go",
    ],
    deps = [
        "//kythe/go/platform/delimited",
    ],
)

go_binary(
    name = "viewindex",
    srcs = [
        "viewindex/viewindex.go",
    ],
    deps = [
        "//kythe/go/platform/kindex",
    ],
)

go_binary(
    name = "indexpack",
    srcs = [
        "indexpack/indexpack.go",
    ],
    deps = [
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/indexpack/google",
        "//kythe/go/platform/kindex",
        "//kythe/go/util/oauth2",
        "//kythe/proto:analysis_proto",
        "//third_party/go:cloud_storage",
        "//third_party/go:context",
    ],
)
