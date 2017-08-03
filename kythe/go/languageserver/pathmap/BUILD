load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])


go_package_library(
    name = "pathmap",
    srcs = [
        "mapper.go",
    ],
)

go_test(
    name = "pathmap_test",
    size = "small",
    srcs = ["mapper_test.go"],
    library = "pathmap",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
    ],
)