load("@io_kythe//tools:build_rules/shims.bzl", "go_binary")

go_binary(
    name = "extract_proto_kzip",
    srcs = ["extract_proto_kzip.go"],
    visibility = ["//visibility:public"],
    deps = [
        "@io_kythe//kythe/go/extractors/bazel",
        "@io_kythe//kythe/go/extractors/bazel/extutil",
        "@io_kythe//kythe/go/util/vnameutil",
        "@io_kythe//kythe/proto:analysis_go_proto",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
)
