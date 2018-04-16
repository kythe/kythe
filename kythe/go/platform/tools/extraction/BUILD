load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "parseconfig",
    srcs = ["parseconfig.go"],
    deps = [
        "//kythe/go/extractors/config/parser",
        "//kythe/proto:extraction_config_go_proto",
        "@go_protobuf//:jsonpb",
        "@go_protobuf//:proto",
    ],
)
