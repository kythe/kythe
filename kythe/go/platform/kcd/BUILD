load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "kcd",
    srcs = ["kcd.go"],
    importpath = "kythe.io/kythe/go/platform/kcd",
    deps = ["//kythe/proto:storage_go_proto"],
)

go_test(
    name = "kcd_test",
    size = "small",
    srcs = ["kcd_test.go"],
    library = ":kcd",
    visibility = ["//visibility:private"],
)
