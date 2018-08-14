load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "cmakecmd",
    srcs = ["cmakecmd.go"],
    deps = [
        "//kythe/go/util/cmdutil",
        "@com_github_google_subcommands//:go_default_library",
        "@com_github_pborman_uuid//:go_default_library",
        "@org_bitbucket_creachadair_shell//:go_default_library",
        "@org_golang_x_sync//errgroup:go_default_library",
        "@org_golang_x_sync//semaphore:go_default_library",
    ],
)
