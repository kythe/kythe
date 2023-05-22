load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "local",
    srcs = ["local.go"],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/analysis/driver",
        "//kythe/go/platform/kzip",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/log",
        "//kythe/proto:analysis_go_proto",
    ],
)
