load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "config",
    srcs = ["config_parser.go"],
    deps = [
        "//kythe/proto:extraction_config_go_proto",
        "@go_protobuf//:jsonpb",
        "@go_protobuf//:proto",
        "@go_shell//:shell",
    ],
)

go_test(
    name = "config_test",
    srcs = ["config_parser_test.go"],
    data = glob(["testdata/**"]),
    library = ":config",
    deps = [
        "//kythe/proto:extraction_config_go_proto",
        "@go_cmp//:cmp",
        "@go_protobuf//:jsonpb",
        "@go_protobuf//:proto",
    ],
)

go_library(
    name = "extractor",
    srcs = ["extractor.go"],
    deps = [
        ":config",
    ],
)
