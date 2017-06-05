load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "remote",
    srcs = ["remote.go"],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:analysis_service_proto_go",
        "@go_errors//:errors",
    ],
)
