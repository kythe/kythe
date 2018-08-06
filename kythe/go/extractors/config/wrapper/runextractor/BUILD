load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "runextractor",
    srcs = ["runextractor.go"],
    deps = [
        "//kythe/go/extractors/config/wrapper",
    ],
)
