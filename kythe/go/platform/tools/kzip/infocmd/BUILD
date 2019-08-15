load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "infocmd",
    srcs = [
        "infocmd.go",
    ],
    deps = [
        ":info",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/cmdutil",
        "@com_github_golang_protobuf//jsonpb:go_default_library_gen",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_google_subcommands//:go_default_library",
    ],
)

go_library(
    name = "info",
    srcs = [
        "info.go",
    ],
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/proto:analysis_go_proto",
    ],
)
