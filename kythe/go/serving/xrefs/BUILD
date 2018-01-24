load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "xrefs",
    srcs = ["xrefs.go"],
    deps = [
        "//kythe/go/services/xrefs",
        "//kythe/go/storage/table",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/tickets",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:serving_go_proto",
        "//kythe/proto:xref_go_proto",
        "@go_grpc//:codes",
        "@go_grpc//:status",
        "@go_protobuf//:proto",
        "@go_snappy//:snappy",
        "@go_stringset//:stringset",
        "@go_x_net//:trace",
    ],
)

go_test(
    name = "xrefs_test",
    size = "small",
    srcs = ["xrefs_test.go"],
    library = "xrefs",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "@go_x_text//:encoding",
        "@go_x_text//:encoding/unicode",
        "@go_x_text//:transform",
    ],
)
