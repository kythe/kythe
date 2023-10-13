load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "vfs",
    srcs = ["vfs.go"],
    importpath = "kythe.io/kythe/go/platform/vfs",
)

go_test(
    name = "vfs_test",
    size = "small",
    srcs = ["vfs_test.go"],
    library = ":vfs",
    deps = [
        "//kythe/go/test/testutil",
    ],
)
