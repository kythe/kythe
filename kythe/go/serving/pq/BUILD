load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "@go_pq//:pq",
        "@go_protobuf//:proto",
        "@go_x_net//:context",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/api",
        "//kythe/go/serving/xrefs/assemble",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:common_proto_go",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:internal_proto_go",
        "//kythe/proto:serving_proto_go",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
    ],
)
