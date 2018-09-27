load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "web",
    srcs = ["web.go"],
    deps = [
        "//kythe/go/util/httpencoding",
        "@com_github_golang_protobuf//jsonpb:go_default_library_gen",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
