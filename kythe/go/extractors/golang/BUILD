load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "golang",
    srcs = ["golang.go"],
    deps = [
        "//kythe/go/extractors/govname",
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/ptypes",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:go_proto_go",
        "//kythe/proto:storage_proto_go",
        "@go_stringset//:stringset",
    ],
)
