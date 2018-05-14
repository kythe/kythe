load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "text",
    srcs = ["text.go"],
    deps = [
        "@org_golang_x_text//encoding:go_default_library",
        "@org_golang_x_text//encoding/htmlindex:go_default_library",
        "@org_golang_x_text//transform:go_default_library",
    ],
)
