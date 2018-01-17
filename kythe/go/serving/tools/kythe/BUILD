load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "kythe",
    srcs = ["kythe.go"],
    deps = [
        "//kythe/go/services/cli",
        "//kythe/go/serving/api",
    ],
)
