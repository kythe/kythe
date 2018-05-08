load("//tools:build_rules/shims.bzl", "go_library")

package(
    default_visibility = ["//kythe:default_visibility"],
    licenses = ["notice"],
)

go_library(
    name = "explore",
    srcs = ["explore.go"],
    deps = [
        "//kythe/go/services/explore",
        "//kythe/go/storage/table",
        "//kythe/proto:explore_go_proto",
        "//kythe/proto:serving_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@go_grpc//:codes",
        "@go_grpc//:status",
    ],
)
