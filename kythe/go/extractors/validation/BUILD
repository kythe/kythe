load("//tools:build_rules/shims.bzl", "go_test", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "validation",
    srcs = [
        "validation.go",
    ],
    deps = [
        "//kythe/go/platform/kzip",
    ],
)

go_test(
    name = "validation_test",
    srcs = ["validation_test.go"],
    data = [],
    library = ":validation",
    deps = [
    ],
)
