load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "extractschema",
    srcs = ["extractschema.go"],
    deps = ["@go_stringset//:stringset"],
)
