load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "parser",
    srcs = ["parser.go"],
    deps = [
        "//kythe/proto:extraction_config_go_proto",
        "@go_shell//:shell",
    ],
)

go_test(
    name = "parser_test",
    srcs = ["parser_test.go"],
    data = glob(["testdata/**"]),
    library = ":parser",
    deps = [
        "//kythe/proto:extraction_config_go_proto",
        "@go_cmp//:cmp",
        "@go_protobuf//:jsonpb",
    ],
)
