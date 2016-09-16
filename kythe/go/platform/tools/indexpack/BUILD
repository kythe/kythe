load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "indexpack",
    srcs = ["indexpack.go"],
    deps = [
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/build",
        "//kythe/proto:analysis_proto_go",
        "@go_gcloud//:storage",
        "@go_x_net//:context",
    ],
)
