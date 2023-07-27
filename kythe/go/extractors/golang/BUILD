load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "golang",
    srcs = [
        "golang.go",
        "packages.go",
    ],
    importpath = "kythe.io/kythe/go/extractors/golang",
    deps = [
        "//kythe/go/extractors/govname",
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/kzip",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/log",
        "//kythe/go/util/ptypes",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//ptypes/any",
        "@org_bitbucket_creachadair_stringset//:stringset",
    ],
)
