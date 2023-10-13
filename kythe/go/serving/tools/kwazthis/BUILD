load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "kwazthis",
    srcs = ["kwazthis.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/services/graph",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/api",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/go/util/schema",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/tickets",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:storage_go_proto",
        "//kythe/proto:xref_go_proto",
    ],
)
