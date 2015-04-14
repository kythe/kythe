load("//tools/build_rules/go", "go_binary")

package(default_visibility = ["//visibility:public"])

go_binary(
    name = "http_server",
    srcs = [
        "http_server/http_server.go",
    ],
    deps = [
        "//kythe/go/services/filetree",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/search",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/search",
        "//kythe/go/serving/xrefs",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/table",
        "//kythe/go/storage/xrefs",
        "//kythe/proto:filetree_proto",
        "//kythe/proto:storage_proto",
        "//kythe/proto:xref_proto",
        "//third_party/go:grpc",
    ],
)

go_binary(
    name = "kythe",
    srcs = [
        "kythe/kythe.go",
        "kythe/kythe_commands.go",
        "kythe/kythe_display.go",
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
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:filetree_proto",
        "//kythe/proto:serving_proto",
        "//kythe/proto:storage_proto",
        "//kythe/proto:xref_proto",
        "//third_party/go:context",
        "//third_party/go:grpc",
    ],
)

go_binary(
    name = "write_tables",
    srcs = [
        "write_tables/write_tables.go",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/serving/pipeline",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/leveldb",
    ],
)
