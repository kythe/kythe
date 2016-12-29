load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "gsutil",
    srcs = ["gsutil.go"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/inmemory",
    ],
)
