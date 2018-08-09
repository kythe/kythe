load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "mavencmd",
    srcs = ["mavencmd.go"],
    deps = [
        "//kythe/go/extractors/config/runextractor/backup",
        "//kythe/go/util/cmdutil",
        "@com_github_google_subcommands//:go_default_library",
    ],
)
