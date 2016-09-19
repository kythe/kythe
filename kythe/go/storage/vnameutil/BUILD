load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "vnameutil",
    srcs = ["rewrite.go"],
    deps = ["//kythe/proto:storage_proto_go"],
)

go_test(
    name = "vnameutil_test",
    srcs = ["rewrite_test.go"],
    library = "vnameutil",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)
