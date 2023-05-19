load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "filetree",
    srcs = ["filetree.go"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/web",
        "//kythe/go/util/log",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/proto:filetree_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)
