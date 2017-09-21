load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "pipeline",
    srcs = ["pipeline.go"],
    deps = [
        "//kythe/go/services/filetree",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/graph",
        "//kythe/go/serving/xrefs",
        "//kythe/go/serving/xrefs/assemble",
        "//kythe/go/storage/keyvalue",
        "//kythe/go/storage/stream",
        "//kythe/go/storage/table",
        "//kythe/go/util/disksort",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/go/util/sortutil",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:internal_proto_go",
        "//kythe/proto:serving_proto_go",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)
