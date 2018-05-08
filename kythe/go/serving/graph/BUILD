load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "graph",
    srcs = ["graph.go"],
    deps = [
        "//kythe/go/services/xrefs",
        "//kythe/go/storage/table",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:serving_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@go_stringset//:stringset",
        "@go_x_net//:trace",
    ],
)

go_test(
    name = "edges_test",
    size = "small",
    srcs = ["graph_test.go"],
    library = "graph",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/go/util/kytheuri",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:serving_go_proto",
        "@go_x_text//:encoding",
        "@go_x_text//:encoding/unicode",
        "@go_x_text//:transform",
    ],
)
