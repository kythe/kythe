load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "web",
    srcs = ["web.go"],
    deps = [
        "//kythe/go/util/httpencoding",
        "@go_protobuf//:jsonpb",
        "@go_protobuf//:proto",
    ],
)
