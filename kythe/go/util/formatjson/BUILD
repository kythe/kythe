load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "formatjson",
    srcs = ["formatjson.go"],
    deps = ["//kythe/go/util/log"],
)
