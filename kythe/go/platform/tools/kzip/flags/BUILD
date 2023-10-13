load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "flags",
    srcs = ["encoding.go"],
    importpath = "kythe.io/kythe/go/platform/tools/kzip/flags",
    deps = [
        "//kythe/go/platform/kzip",
    ],
)
