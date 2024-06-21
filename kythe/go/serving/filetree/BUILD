load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "filetree",
    srcs = ["filetree.go"],
    importpath = "kythe.io/kythe/go/serving/filetree",
    deps = [
        "//kythe/go/storage/table",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/proto:filetree_go_proto",
        "//kythe/proto:serving_go_proto",
    ],
)
