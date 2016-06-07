load("//tools:build_rules/go.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "analyzer_driver",
    srcs = glob(["analyzer_driver/*.go"]),
    deps = [
        "@go_grpc//:grpc",
        "@go_protobuf//:proto",
        "@go_x_net//:context",
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/analysis/driver",
        "//kythe/go/platform/analysis/local",
        "//kythe/go/platform/analysis/remote",
        "//kythe/go/platform/delimited",
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/netutil",
        "//kythe/go/util/process",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
    ],
)

sh_test(
    name = "analyzer_driver_test",
    size = "small",
    srcs = ["testdata/analyzer_driver_test.sh"],
    data = [
        ":analyzer_driver",
        "//kythe/go/platform/tools:entrystream",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer_server",
        "//kythe/testdata:test.kindex",
    ],
    tags = [
        "flaky",  # flaky due to port picking
        "local",
    ],
)

go_binary(
    name = "entrystream",
    srcs = ["entrystream/entrystream.go"],
    deps = [
        "@go_protobuf//:proto",
        "//kythe/go/platform/delimited",
        "//kythe/go/services/graphstore/compare",
        "//kythe/go/storage/stream",
        "//kythe/go/util/disksort",
        "//kythe/go/util/flagutil",
        "//kythe/proto:storage_proto_go",
    ],
)

go_binary(
    name = "ktags",
    srcs = ["ktags/ktags.go"],
    deps = [
        "@go_x_net//:context",
        "//kythe/go/services/search",
        "//kythe/go/services/xrefs",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
    ],
)

go_binary(
    name = "dedup_stream",
    srcs = ["dedup_stream/dedup_stream.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/util/datasize",
        "//kythe/go/util/flagutil",
    ],
)

go_binary(
    name = "viewindex",
    srcs = ["viewindex/viewindex.go"],
    deps = [
        "@go_x_net//:context",
        "//kythe/go/platform/kindex",
        "//kythe/go/util/flagutil",
    ],
)

go_binary(
    name = "indexpack",
    srcs = ["indexpack/indexpack.go"],
    deps = [
        "@go_gcloud//:storage",
        "@go_x_net//:context",
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/platform/vfs",
        "//kythe/go/platform/vfs/gcs",
        "//kythe/go/util/build",
        "//kythe/go/util/oauth2",
        "//kythe/proto:analysis_proto_go",
    ],
)

sh_test(
    name = "indexpack_test",
    size = "small",
    srcs = ["indexpack_test.sh"],
    data = [
        ":indexpack",
        ":viewindex",
        "//external:jq",
        "//kythe/testdata:test.kindex",
    ],
)
