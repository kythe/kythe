load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "bazel_go_extractor",
    srcs = ["bazel_go_extractor.go"],
    deps = [
        "//kythe/go/extractors/bazel",
        "//kythe/go/extractors/govname",
        "//kythe/go/util/log",
        "//kythe/go/util/ptypes",
        "//kythe/go/util/vnameutil",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:go_go_proto",
    ],
)
