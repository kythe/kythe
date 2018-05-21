load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "cmdutil",
    srcs = ["cmdutil.go"],
    deps = ["@com_github_google_subcommands//:go_default_library"],
)

go_test(
    name = "cmdutil_test",
    srcs = ["cmdutil_test.go"],
    testonly = True,
    deps = [
        ":cmdutil",
        "@com_github_google_subcommands//:go_default_library",
    ],
)
