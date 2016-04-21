load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
    deps = [
        "@go_x_net//:context",
        "//kythe/go/services/graphstore",
        "//kythe/go/util/datasize",
        "//kythe/proto:storage_proto_go",
    ],
)
