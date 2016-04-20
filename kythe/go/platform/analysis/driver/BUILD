load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "//kythe/go/test/testutil",
        "//kythe/proto:storage_proto_go",
    ],
    deps = [
        "@go_x_net//:context",
        "//kythe/go/platform/analysis",
        "//kythe/proto:analysis_proto_go",
    ],
)
