load("@io_bazel_rules_go//go:def.bzl", "go_library")

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
