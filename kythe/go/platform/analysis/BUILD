load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "analysis",
    srcs = ["analysis.go"],
    deps = [
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:analysis_service_proto_go",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)
