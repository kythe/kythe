load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "api",
    srcs = ["api.go"],
    deps = [
        "//kythe/go/services/filetree",
        "//kythe/go/services/graph",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/graph",
        "//kythe/go/serving/identifiers",
        "//kythe/go/serving/xrefs",
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/table",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:graph_proto_go",
        "//kythe/proto:identifier_proto_go",
        "//kythe/proto:xref_proto_go",
    ],
)
