load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "gradlecmd",
    srcs = [
        "gradlecmd.go",
    ],
    deps = [
        "//kythe/go/extractors/config/preprocessor/modifier",
        "//kythe/go/extractors/config/runextractor/backup",
        "//kythe/go/extractors/constants",
        "//kythe/go/util/cmdutil",
        "@com_github_google_subcommands//:go_default_library",
    ],
)
