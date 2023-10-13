load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "graph",
    srcs = ["graph.go"],
    importpath = "kythe.io/kythe/go/services/graph",
    deps = [
        "//kythe/go/services/web",
        "//kythe/go/util/log",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:graph_go_proto",
    ],
)
