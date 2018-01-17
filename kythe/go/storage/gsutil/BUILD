load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "gsutil",
    srcs = ["gsutil.go"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/inmemory",
    ],
)
