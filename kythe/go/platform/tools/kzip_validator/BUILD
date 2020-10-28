load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "kzip_validator",
    srcs = ["kzip_validator.go"],
    deps = [
        "//kythe/go/extractors/validation",
        "@com_github_mholt_archiver//:go_default_library",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
)
