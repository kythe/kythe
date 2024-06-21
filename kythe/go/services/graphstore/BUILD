load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "graphstore",
    srcs = ["graphstore.go"],
    importpath = "kythe.io/kythe/go/services/graphstore",
    deps = [
        "//kythe/go/util/compare",
        "//kythe/proto:storage_go_proto",
    ],
)
