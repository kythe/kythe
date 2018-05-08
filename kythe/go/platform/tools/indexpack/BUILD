load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "indexpack",
    srcs = ["indexpack.go"],
    deps = [
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/build",
        "//kythe/proto:analysis_go_proto",
    ],
)
