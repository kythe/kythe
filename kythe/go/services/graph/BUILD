load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "graph",
    srcs = ["graph.go"],
    deps = [
        "//kythe/go/services/web",
        "//kythe/proto:common_proto_go",
        "//kythe/proto:graph_proto_go",
    ],
)
