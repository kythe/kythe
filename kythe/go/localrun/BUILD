load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "localrun",
    srcs = glob(["*.go"]),
    deps = [
        "@org_golang_x_sync//errgroup:go_default_library",
    ],
)
