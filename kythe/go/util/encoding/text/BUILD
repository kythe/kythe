load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "text",
    srcs = ["text.go"],
    deps = [
        "@go_x_text//:encoding",
        "@go_x_text//:encoding/htmlindex",
        "@go_x_text//:transform",
    ],
)
