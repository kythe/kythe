load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "graph",
    srcs = ["graph.go"],
    deps = [
        "//kythe/go/services/xrefs",
        "//kythe/go/storage/table",
        "//kythe/proto:common_proto_go",
        "//kythe/proto:graph_proto_go",
        "//kythe/proto:internal_proto_go",
        "//kythe/proto:serving_proto_go",
        "@go_protobuf//:proto",
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
        "@go_x_text//:encoding",
        "@go_x_text//:encoding/unicode",
        "@go_x_text//:transform",
    ],
)
