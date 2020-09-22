load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "extract_kzip",
    srcs = ["extract_kzip.go"],
    deps = [
        "//kythe/go/extractors/bazel",
        "//kythe/go/extractors/bazel/extutil",
        "//kythe/go/platform/kindex",
        "//kythe/go/util/vnameutil",
        "//third_party/bazel:extra_actions_base_go_proto",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)
