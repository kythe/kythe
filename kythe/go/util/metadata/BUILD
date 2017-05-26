load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "metadata",
    srcs = ["metadata.go"],
    deps = [
        "//kythe/go/util/schema/edges",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:protoc-gen-go/descriptor",
    ],
)

go_test(
    name = "metadata_test",
    size = "small",
    srcs = ["metadata_test.go"],
    library = ":metadata",
    deps = [
        "//kythe/go/test/testutil",
        "@go_protobuf//:proto",
    ],
)
