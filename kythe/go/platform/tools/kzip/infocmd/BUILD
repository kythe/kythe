load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "infocmd",
    srcs = [
        "infocmd.go",
    ],
    deps = [
        "//kythe/go/platform/kindex",
        "//kythe/go/platform/kzip",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/cmdutil",
        "//kythe/proto:analysis_go_proto",
        "@com_github_golang_protobuf//jsonpb:go_default_library_gen",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_google_subcommands//:go_default_library",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
)
