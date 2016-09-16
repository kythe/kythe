load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "viewindex",
    srcs = ["viewindex.go"],
    deps = [
        "//kythe/go/platform/kindex",
        "//kythe/go/util/flagutil",
        "@go_x_net//:context",
    ],
)
