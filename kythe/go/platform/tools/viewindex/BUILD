load("//tools:build_rules/go.bzl", "go_release_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_release_binary(
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
