load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "validation",
    srcs = [
        "validation.go",
    ],
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/proto:analysis_go_proto",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
)
