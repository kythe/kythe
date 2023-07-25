load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "cache",
    srcs = ["cache.go"],
    importpath = "kythe.io/kythe/go/platform/cache",
    deps = ["//kythe/go/platform/analysis"],
)

go_test(
    name = "cache_test",
    size = "small",
    srcs = ["cache_test.go"],
    library = ":cache",
    visibility = ["//visibility:private"],
)
