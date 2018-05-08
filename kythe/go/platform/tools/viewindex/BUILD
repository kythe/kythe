load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "viewindex",
    srcs = ["viewindex.go"],
    deps = [
        "//kythe/go/platform/kindex",
        "//kythe/go/util/flagutil",
        "//kythe/proto:buildinfo_go_proto",
        "//kythe/proto:cxx_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:java_go_proto",
        "@com_github_golang_protobuf//jsonpb:go_default_library",
    ],
)
