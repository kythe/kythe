load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "testutil",
    srcs = ["testutil.go"],
    deps = [
        "//kythe/go/platform/kcd",
        "//kythe/go/platform/kcd/kythe",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)
