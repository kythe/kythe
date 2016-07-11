load("//tools:build_rules/go.bzl", "go_binary")

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
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
        "@go_x_net//:context",
    ],
)
