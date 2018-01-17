load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "keyvalue",
    srcs = ["keyvalue.go"],
    deps = [
        "//kythe/go/storage/keyvalue",
        "//kythe/go/test/testutil",
    ],
)
