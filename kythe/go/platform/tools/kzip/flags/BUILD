load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "flags",
    srcs = ["encoding.go"],
    deps = [
        "//kythe/go/platform/kzip",
    ],
)

go_test(
    name = "flags_test",
    size = "small",
    srcs = [
        "encoding_test.go",
    ],
    library = ":flags",
    deps = [
        "//kythe/go/platform/kzip",
    ]
)
