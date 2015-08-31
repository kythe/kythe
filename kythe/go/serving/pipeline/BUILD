load("/tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "//kythe/go/services/filetree",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/compare",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/search",
        "//kythe/go/serving/xrefs",
        "//kythe/go/serving/xrefs/assemble",
        "//kythe/go/storage/keyvalue",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/table",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:serving_proto_go",
        "//kythe/proto:storage_proto_go",
        "//third_party/go:context",
        "//third_party/go:protobuf",
    ],
)
