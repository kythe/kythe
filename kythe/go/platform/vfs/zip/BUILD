load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "zip",
    srcs = ["fs.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "@go_x_net//:context",
    ],
)
