load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "golang",
    srcs = [
        "golang.go",
        "packages.go",
    ],
    deps = [
        "//kythe/go/extractors/govname",
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/kzip",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/ptypes",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:storage_go_proto",
        "@io_bazel_rules_go//proto/wkt:any_go_proto",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
)
