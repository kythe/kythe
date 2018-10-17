load("//tools:build_rules/shims.bzl", "go_binary", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "preprocessor",
    srcs = ["preprocessor.go"],
    deps = [
        "//kythe/go/extractors/config/constants",
        "//kythe/go/extractors/config/preprocessor/modifier:gradlemod",
        "//kythe/go/extractors/config/preprocessor/modifier:mvnmod",
    ],
)
