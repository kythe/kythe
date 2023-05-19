load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "yamlgen",
    srcs = ["yamlgen.go"],
    deps = [
        "//kythe/go/extractors/gcp/config",
        "//kythe/go/util/log",
    ],
)
