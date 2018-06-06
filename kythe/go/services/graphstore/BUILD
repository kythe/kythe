load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "graphstore",
    srcs = ["graphstore.go"],
    deps = [
        "//kythe/go/util/compare",
        "//kythe/proto:storage_go_proto",
        "//kythe/proto:storage_service_go_proto",
    ],
)
