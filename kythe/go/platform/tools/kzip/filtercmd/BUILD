load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "filtercmd",
    srcs = ["filtercmd.go"],
    importpath = "kythe.io/kythe/go/platform/tools/kzip/filtercmd",
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/go/platform/tools/kzip/flags",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/cmdutil",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/log",
        "@com_github_google_subcommands//:subcommands",
        "@org_bitbucket_creachadair_stringset//:stringset",
    ],
)
