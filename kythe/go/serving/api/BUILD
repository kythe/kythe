load("/tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "//kythe/go/services/filetree",
        "//kythe/go/services/search",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/search",
        "//kythe/go/serving/xrefs",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/table",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
        "//third_party/go/src/golang.org/x/net/context",
        "//third_party/go/src/google.golang.org/grpc",
    ],
)
