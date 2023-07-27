load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "cmdutil",
    srcs = ["cmdutil.go"],
    importpath = "kythe.io/kythe/go/util/cmdutil",
    deps = ["@com_github_google_subcommands//:subcommands"],
)

go_test(
    name = "cmdutil_test",
    testonly = True,
    srcs = ["cmdutil_test.go"],
    deps = [
        ":cmdutil",
        "@com_github_google_subcommands//:subcommands",
    ],
)
