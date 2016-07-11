load("//tools:build_rules/go.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "dedup_stream",
    srcs = ["dedup_stream.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/util/datasize",
        "//kythe/go/util/flagutil",
    ],
)
