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
        "@com_github_google_subcommands//:go_default_library",
        "@org_golang_google_protobuf//encoding/protojson:go_default_library",
        "@org_golang_google_protobuf//encoding/prototext:go_default_library",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)
