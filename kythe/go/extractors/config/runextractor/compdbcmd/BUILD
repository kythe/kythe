load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "compdbcmd",
    srcs = ["compdbcmd.go"],
    importpath = "kythe.io/kythe/go/extractors/config/runextractor/compdbcmd",
    deps = [
        "//kythe/go/extractors/config/runextractor/compdb",
        "//kythe/go/util/cmdutil",
        "@com_github_google_subcommands//:subcommands",
    ],
)
