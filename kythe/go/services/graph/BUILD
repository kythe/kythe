load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "graph",
    srcs = ["graph.go"],
    deps = [
        "//kythe/go/services/web",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:graph_go_proto",
    ],
)
