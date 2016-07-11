load("//tools:build_rules/go.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "indexpack",
    srcs = ["indexpack.go"],
    deps = [
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/platform/vfs",
        "//kythe/go/platform/vfs/gcs",
        "//kythe/go/util/build",
        "//kythe/go/util/oauth2",
        "//kythe/proto:analysis_proto_go",
        "@go_gcloud//:storage",
        "@go_x_net//:context",
    ],
)
