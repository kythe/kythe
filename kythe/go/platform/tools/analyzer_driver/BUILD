load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "analyzer_driver",
    srcs = ["analyzer_driver.go"],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/analysis/driver",
        "//kythe/go/platform/analysis/local",
        "//kythe/go/platform/analysis/remote",
        "//kythe/go/platform/delimited",
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/netutil",
        "//kythe/go/util/process",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:analysis_service_proto_go",
        "@go_grpc//:grpc",
        "@go_protobuf//:proto",
        "@go_x_net//:context",
    ],
)
