load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "pager",
    srcs = ["pager.go"],
    importpath = "kythe.io/kythe/go/util/pager",
    deps = ["//kythe/go/util/sortutil"],
)

go_test(
    name = "pager_test",
    size = "small",
    srcs = ["pager_test.go"],
    library = ":pager",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
