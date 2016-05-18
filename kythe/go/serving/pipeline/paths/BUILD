load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = ["//kythe/go/test/testutil"],
    deps = [
        "@go_protobuf//:proto",
        "@go_x_net//:context",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/xrefs/assemble",
        "//kythe/go/util/reduce",
        "//kythe/go/util/schema",
        "//kythe/proto:common_proto_go",
        "//kythe/proto:internal_proto_go",
        "//kythe/proto:serving_proto_go",
    ],
)
