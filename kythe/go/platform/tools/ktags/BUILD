load("//tools:build_rules/go.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "ktags",
    srcs = ["ktags.go"],
    deps = [
        "//kythe/go/services/xrefs",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:xref_proto_go",
        "@go_x_net//:context",
    ],
)
