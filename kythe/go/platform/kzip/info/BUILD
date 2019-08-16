load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "info",
    srcs = [
        "info.go",
    ],
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/proto:analysis_go_proto",
    ],
)
