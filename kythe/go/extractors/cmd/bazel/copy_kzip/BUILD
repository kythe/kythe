load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "copy_kzip",
    srcs = ["copy_kzip.go"],
    deps = [
        "//kythe/go/extractors/bazel",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/log",
    ],
)
