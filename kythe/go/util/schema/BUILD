load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "schema",
    srcs = ["schema.go"],
    deps = [
        "//kythe/go/util/schema/facts",
        "//kythe/proto:storage_proto_go",
    ],
)

go_test(
    name = "schema_test",
    size = "small",
    srcs = ["schema_test.go"],
    library = "schema",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/util/schema/nodes",
        "@go_protobuf//:proto",
    ],
)
