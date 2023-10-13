load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "api",
    srcs = ["api.go"],
    importpath = "kythe.io/kythe/go/serving/api",
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
        "//kythe/proto:filetree_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:identifier_go_proto",
        "//kythe/proto:xref_go_proto",
    ],
)
