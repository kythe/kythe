load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "packdb",
    srcs = ["packdb.go"],
    deps = [
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kcd",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "packdb_test",
    size = "small",
    srcs = ["packdb_test.go"],
    library = "packdb",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/platform/kcd/kythe",
        "//kythe/go/platform/kcd/testutil",
    ],
)
