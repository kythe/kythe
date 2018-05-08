load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "metadata",
    srcs = ["metadata.go"],
    deps = [
        "//kythe/go/util/schema/edges",
        "//kythe/proto:storage_go_proto",
        "@io_bazel_rules_go//proto/wkt:descriptor_go_proto",
    ],
)

go_test(
    name = "metadata_test",
    size = "small",
    srcs = ["metadata_test.go"],
    library = ":metadata",
    deps = [
        "//kythe/go/test/testutil",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
