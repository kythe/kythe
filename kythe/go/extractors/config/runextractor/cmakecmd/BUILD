load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "cmakecmd",
    srcs = ["cmakecmd.go"],
    importpath = "kythe.io/kythe/go/extractors/config/runextractor/cmakecmd",
    deps = [
        "//kythe/go/extractors/config/runextractor/compdb",
        "//kythe/go/util/cmdutil",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/log",
        "@com_github_google_subcommands//:subcommands",
    ],
)
