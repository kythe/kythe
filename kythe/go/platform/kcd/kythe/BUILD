load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "kythe",
    srcs = ["units.go"],
    deps = [
        "//kythe/go/platform/kcd",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "kythe_test",
    srcs = ["units_test.go"],
    library = "kythe",
    visibility = ["//visibility:private"],
)
