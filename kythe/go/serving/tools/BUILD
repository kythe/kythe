load("//tools:build_rules/go.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "pq",
    srcs = ["pq/pq.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/services/filetree",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/pq",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/stream",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/profile",
        "//kythe/go/util/schema",
        "//kythe/proto:storage_proto_go",
        "//third_party/go/src/golang.org/x/net/context",
    ],
)

go_binary(
    name = "http_server",
    srcs = ["http_server/http_server.go"],
    deps = [
        "//kythe/go/services/filetree",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/grpc",
        "//kythe/go/services/graphstore/proxy",
        "//kythe/go/services/search",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/search",
        "//kythe/go/serving/xrefs",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/table",
        "//kythe/go/storage/xrefs",
        "//kythe/go/util/flagutil",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
        "//third_party/go/src/golang.org/x/net/context",
        "//third_party/go/src/golang.org/x/net/http2",
        "//third_party/go/src/google.golang.org/grpc",
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
        "//kythe/go/platform/vfs",
        "//kythe/go/services/filetree",
        "//kythe/go/services/search",
        "//kythe/go/services/web",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/api",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
        "//third_party/go/src/github.com/golang/protobuf/proto",
        "//third_party/go/src/golang.org/x/net/context",
    ],
)

go_binary(
    name = "kwazthis",
    srcs = glob(["kwazthis/*.go"]),
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/services/search",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/api",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
        "//third_party/go/src/golang.org/x/net/context",
    ],
)

go_binary(
    name = "write_tables",
    srcs = ["write_tables/write_tables.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/grpc",
        "//kythe/go/services/graphstore/proxy",
        "//kythe/go/serving/pipeline",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/stream",
        "//kythe/go/util/datasize",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/profile",
        "//kythe/proto:storage_proto_go",
        "//third_party/go/src/golang.org/x/net/context",
    ],
)
