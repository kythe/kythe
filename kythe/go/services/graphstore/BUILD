load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "graphstore",
    srcs = ["graphstore.go"],
    deps = [
        "//kythe/go/services/graphstore/compare",
        "//kythe/proto:storage_go_proto",
        "//kythe/proto:storage_service_go_proto",
    ],
)
