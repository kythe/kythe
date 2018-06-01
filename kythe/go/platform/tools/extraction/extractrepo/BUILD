load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "extractrepo",
    srcs = ["extractrepo.go"],
    deps = ["//kythe/go/extractors/config"],
)
