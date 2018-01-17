load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "filetree",
    srcs = ["filetree.go"],
    deps = [
        "//kythe/go/storage/table",
        "//kythe/proto:filetree_go_proto",
        "//kythe/proto:serving_go_proto",
    ],
)
