load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "pager",
    srcs = ["pager.go"],
    deps = [
        "//kythe/go/util/sortutil",
    ],
)

go_test(
    name = "pager_test",
    size = "small",
    srcs = ["pager_test.go"],
    library = "pager",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
