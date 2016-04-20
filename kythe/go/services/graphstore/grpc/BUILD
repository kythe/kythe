load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "@go_grpc//:grpc",
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/gsutil",
        "//kythe/proto:storage_proto_go",
    ],
)
