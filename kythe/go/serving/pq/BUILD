load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "pq",
    srcs = [
        "pq.go",
        ":pq_build.go",
        ":pq_filetree.go",
        ":pq_xrefs.go",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/api",
        "//kythe/go/serving/xrefs/assemble",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:common_proto_go",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:internal_proto_go",
        "//kythe/proto:serving_proto_go",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
        "@go_pq//:pq",
        "@go_protobuf//:proto",
        "@go_stringset//:stringset",
        "@go_x_net//:context",
    ],
)
