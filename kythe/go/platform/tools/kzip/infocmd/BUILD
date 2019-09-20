load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "infocmd",
    srcs = [
        "infocmd.go",
    ],
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/go/platform/kzip/info",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/cmdutil",
        "@com_github_golang_protobuf//jsonpb:go_default_library_gen",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_google_subcommands//:go_default_library",
    ],
)
