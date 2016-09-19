load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "graphstore",
    srcs = ["graphstore.go"],
    deps = [
        "//kythe/go/services/graphstore/compare",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:storage_service_proto_go",
        "@go_x_net//:context",
    ],
)
