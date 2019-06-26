load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "infocmd",
    srcs = [
        "infocmd.go",
    ],
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/go/util/cmdutil",
        "@com_github_google_subcommands//:go_default_library",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
)
