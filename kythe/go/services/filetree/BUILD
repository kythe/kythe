load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "filetree",
    srcs = ["filetree.go"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/web",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:storage_proto_go",
    ],
)
