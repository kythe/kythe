load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "gotool",
    srcs = [
        "gotool.go",
    ],
    deps = [
        "//kythe/go/extractors/golang",
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/platform/vfs",
        "//kythe/proto:analysis_proto_go",
        "@go_uuid//:uuid",
        "@go_x_net//:context",
    ],
)
