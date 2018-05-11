load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "extractschema",
    srcs = ["extractschema.go"],
    deps = ["@go_stringset//:stringset"],
)
