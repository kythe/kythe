load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "datasize",
    srcs = ["datasize.go"],
    importpath = "kythe.io/kythe/go/util/datasize",
)

go_test(
    name = "datasize_test",
    size = "small",
    srcs = ["datasize_test.go"],
    library = ":datasize",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
