load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "archive",
    srcs = ["reader.go"],
    importpath = "kythe.io/kythe/go/util/archive",
)

go_test(
    name = "archive_test",
    size = "small",
    srcs = ["reader_test.go"],
    library = ":archive",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/util/log"],
)
