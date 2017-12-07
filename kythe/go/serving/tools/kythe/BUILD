load("//tools:build_rules/go.bzl", "go_release_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_release_binary(
    name = "kythe",
    srcs = ["kythe.go"],
    deps = [
        "//kythe/go/services/cli",
        "//kythe/go/serving/api",
    ],
)
