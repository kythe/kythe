load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "//kythe/go/test/testutil",
    ],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/proto:storage_proto_go",
    ],
)
