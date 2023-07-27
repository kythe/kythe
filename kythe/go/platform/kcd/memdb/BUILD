load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "memdb",
    srcs = ["memdb.go"],
    importpath = "kythe.io/kythe/go/platform/kcd/memdb",
    deps = ["//kythe/go/platform/kcd"],
)

go_test(
    name = "memdb_test",
    size = "small",
    srcs = ["memdb_test.go"],
    library = ":memdb",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/platform/kcd/testutil"],
)
