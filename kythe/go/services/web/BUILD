load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "web",
    srcs = ["web.go"],
    deps = [
        "//kythe/go/util/httpencoding",
        "@go_protobuf//:jsonpb",
        "@go_protobuf//:proto",
    ],
)
