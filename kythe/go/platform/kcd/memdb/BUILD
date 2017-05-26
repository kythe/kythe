load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "memdb",
    srcs = ["memdb.go"],
    deps = [
        "//kythe/go/platform/kcd",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "memdb_test",
    size = "small",
    srcs = ["memdb_test.go"],
    library = "memdb",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/platform/kcd/testutil"],
)
