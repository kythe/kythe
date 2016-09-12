load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:analysis_service_proto_go",
        "@go_x_net//:context",
    ],
)
