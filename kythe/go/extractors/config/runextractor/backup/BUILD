load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "backup",
    srcs = ["backup.go"],
    deps = [
        "//kythe/go/util/log",
        "@com_github_google_go_cmp//cmp:go_default_library",
    ],
)
