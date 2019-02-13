load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "cmakecmd",
    srcs = ["cmakecmd.go"],
    deps = [
        "//kythe/go/extractors/config/runextractor/compdb",
        "//kythe/go/util/cmdutil",
        "@com_github_google_subcommands//:go_default_library",
        "@com_github_google_uuid//:go_default_library",
    ],
)
