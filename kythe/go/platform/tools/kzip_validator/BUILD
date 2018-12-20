load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "kzip_validator",
    srcs = ["kzip_validator.go"],
    deps = [
        "//kythe/go/extractors/validation",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
)
