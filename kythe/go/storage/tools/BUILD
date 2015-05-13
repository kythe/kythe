load("//tools/build_rules/go", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "write_entries",
    srcs = [
        "write_entries/write_entries.go",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/stream",
        "//kythe/proto:storage_proto_go",
    ],
)

go_binary(
    name = "read_entries",
    srcs = [
        "read_entries/read_entries.go",
    ],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/gsutil",
        "//kythe/proto:storage_proto_go",
    ],
)

go_binary(
    name = "triples",
    srcs = [
        "triples/triples.go",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/stream",
        "//kythe/go/util/encoding/rdf",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/proto:storage_proto_go",
    ],
)

go_binary(
    name = "directory_indexer",
    srcs = [
        "directory_indexer/directory_indexer.go",
    ],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/storage/vnameutil",
        "//kythe/proto:storage_proto_go",
    ],
)
