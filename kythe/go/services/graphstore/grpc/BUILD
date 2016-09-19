load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "grpc",
    srcs = ["grpc.go"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/gsutil",
        "//kythe/proto:storage_service_proto_go",
        "@go_grpc//:grpc",
    ],
)
