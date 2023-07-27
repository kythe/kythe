load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "mavencmd",
    srcs = ["mavencmd.go"],
    importpath = "kythe.io/kythe/go/extractors/config/runextractor/mavencmd",
    deps = [
        "//kythe/go/extractors/config/preprocessor/modifier",
        "//kythe/go/extractors/config/runextractor/backup",
        "//kythe/go/extractors/constants",
        "//kythe/go/util/cmdutil",
        "//kythe/go/util/log",
        "@com_github_google_subcommands//:subcommands",
    ],
)
