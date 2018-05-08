load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "graphstore",
    srcs = ["graphstore.go"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/compare",
        "//kythe/go/test/testutil",
        "//kythe/proto:storage_go_proto",
    ],
)
