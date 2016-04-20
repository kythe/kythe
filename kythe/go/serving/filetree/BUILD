load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "@go_x_net//:context",
        "//kythe/go/storage/table",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:serving_proto_go",
    ],
)
