load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "disksort",
    srcs = ["disksort.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/util/sortutil",
        "@go_snappy//:snappy",
    ],
)

go_test(
    name = "disksort_test",
    size = "small",
    srcs = ["disksort_test.go"],
    library = "disksort",
    visibility = ["//visibility:private"],
)
