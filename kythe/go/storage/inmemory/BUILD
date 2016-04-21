load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "@go_protobuf//:proto",
        "@go_x_net//:context",
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/compare",
        "//kythe/proto:storage_proto_go",
    ],
)
