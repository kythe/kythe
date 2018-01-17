load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "text",
    srcs = ["text.go"],
    deps = [
        "@go_x_text//:encoding",
        "@go_x_text//:encoding/htmlindex",
        "@go_x_text//:transform",
    ],
)
