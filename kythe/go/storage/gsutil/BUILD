load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "gsutil",
    srcs = ["gsutil.go"],
    importpath = "kythe.io/kythe/go/storage/gsutil",
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/inmemory",
        "//kythe/go/util/log",
    ],
)
