load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "ktags",
    srcs = ["ktags.go"],
    deps = [
        "//kythe/go/services/graph",
        "//kythe/go/services/xrefs",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:xref_go_proto",
        "@org_bitbucket_creachadair_stringset//:stringset",
    ],
)
