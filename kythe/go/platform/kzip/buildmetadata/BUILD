load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "buildmetadata",
    srcs = ["buildmetadata.go"],
    importpath = "kythe.io/kythe/go/platform/kzip/buildmetadata",
    deps = [
        "//kythe/go/util/ptypes",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//ptypes:go_default_library_gen",
    ],
)
