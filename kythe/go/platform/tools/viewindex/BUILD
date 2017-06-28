load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "viewindex",
    srcs = ["viewindex.go"],
    deps = [
        "//kythe/go/platform/kindex",
        "//kythe/go/util/flagutil",
        "//kythe/proto:buildinfo_proto_go",
        "//kythe/proto:cxx_proto_go",
        "//kythe/proto:go_proto_go",
        "//kythe/proto:java_proto_go",
        "@go_protobuf//:jsonpb",
    ],
)
