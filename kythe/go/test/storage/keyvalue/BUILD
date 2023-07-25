load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "keyvalue",
    srcs = ["keyvalue.go"],
    importpath = "kythe.io/kythe/go/test/storage/keyvalue",
    deps = [
        "//kythe/go/storage/keyvalue",
        "//kythe/go/test/testutil",
    ],
)
