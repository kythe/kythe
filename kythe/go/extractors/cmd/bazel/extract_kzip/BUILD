load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "extract_kzip",
    srcs = ["extract_kzip.go"],
    deps = [
        "//kythe/go/extractors/bazel",
        "//kythe/go/extractors/bazel/extutil",
        "//kythe/go/util/log",
    ],
)
