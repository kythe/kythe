load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "cache",
    srcs = ["cache.go"],
    deps = ["//kythe/go/platform/analysis"],
)

go_test(
    name = "cache_test",
    size = "small",
    srcs = ["cache_test.go"],
    library = "cache",
    visibility = ["//visibility:private"],
)
