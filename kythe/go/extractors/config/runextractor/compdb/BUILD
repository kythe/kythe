load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "compdb",
    srcs = ["compdb.go"],
    deps = [
        "@org_bitbucket_creachadair_shell//:go_default_library",
        "@org_golang_x_sync//errgroup:go_default_library",
        "@org_golang_x_sync//semaphore:go_default_library",
    ],
)
