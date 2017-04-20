load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "kwazthis",
    srcs = ["kwazthis.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/api",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/go/util/schema/tickets",
        "//kythe/proto:common_proto_go",
        "//kythe/proto:graph_proto_go",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
    ],
)
