load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "text",
    srcs = ["text.go"],
    importpath = "kythe.io/kythe/go/util/encoding/text",
    deps = [
        "@org_golang_x_text//encoding",
        "@org_golang_x_text//encoding/htmlindex",
        "@org_golang_x_text//transform",
    ],
)
