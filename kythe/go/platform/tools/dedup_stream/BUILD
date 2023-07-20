load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//visibility:public"])

go_binary(
    name = "dedup_stream",
    srcs = ["dedup_stream.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/platform/delimited/dedup",
        "//kythe/go/util/datasize",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/log",
    ],
)
