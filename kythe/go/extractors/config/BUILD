load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "config_parser",
    srcs = ["config_parser.go"],
    deps = [
        "//kythe/proto:extraction_config_go_proto",
        "@go_shell//:shell",
    ],
)

go_test(
    name = "config_parser_test",
    srcs = ["config_parser_test.go"],
    deps = [
        "//kythe/proto:extraction_config_go_proto",
        "@go_protobuf//:jsonpb",
        "@go_cmp//:cmp",
        ":config_parser",
    ],
    data = glob(["testdata/**"]),
    library = ":config_parser",
)
