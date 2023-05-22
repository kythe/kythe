load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "mkdata",
    srcs = ["mkdata.go"],
    deps = [
        "//kythe/go/util/log",
        "//kythe/proto:schema_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@io_bazel_rules_go//proto/wkt:descriptor_go_proto",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
)
