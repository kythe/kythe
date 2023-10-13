load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "process",
    srcs = ["async.go"],
    importpath = "kythe.io/kythe/go/util/process",
)

go_test(
    name = "process_test",
    size = "small",
    srcs = ["process_test.go"],
    library = ":process",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
