load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "@go_protobuf//:proto",
        "@go_x_net//:context",
        "//kythe/go/services/filetree",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/filetree",
        "//kythe/go/serving/xrefs",
        "//kythe/go/serving/xrefs/assemble",
        "//kythe/go/storage/keyvalue",
        "//kythe/go/storage/stream",
        "//kythe/go/storage/table",
        "//kythe/go/util/disksort",
        "//kythe/go/util/schema",
        "//kythe/go/util/sortutil",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:internal_proto_go",
        "//kythe/proto:serving_proto_go",
        "//kythe/proto:storage_proto_go",
    ],
)
