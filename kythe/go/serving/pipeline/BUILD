load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
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
        "//kythe/proto:filetree_go_proto",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:serving_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
