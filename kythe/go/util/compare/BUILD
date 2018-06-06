load("//tools:build_rules/shims.bzl", "go_test", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "compare",
    srcs = ["compare.go"],
    deps = ["//kythe/proto:storage_go_proto"],
)

go_test(
    name = "compare_test",
    srcs = ["compare_test.go"],
    library = "compare",
)
