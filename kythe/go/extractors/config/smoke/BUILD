load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "smoke",
    srcs = [
        "smoke.go",
    ],
    deps = [
        "//kythe/go/extractors/config",
        "//kythe/go/platform/kindex",
    ],
)
