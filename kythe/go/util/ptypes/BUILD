load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "ptypes",
    srcs = ["ptypes.go"],
    deps = [
        "//third_party/proto:any_proto_go",
        "@go_protobuf//:proto",
        "@go_protobuf//:ptypes",
        "@go_protobuf//:ptypes/any",
    ],
)

go_test(
    name = "ptypes_test",
    size = "small",
    srcs = ["ptypes_test.go"],
    library = "ptypes",
    visibility = ["//visibility:private"],
)
