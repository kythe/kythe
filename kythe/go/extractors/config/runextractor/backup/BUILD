load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "backup",
    srcs = ["backup.go"],
    importpath = "kythe.io/kythe/go/extractors/config/runextractor/backup",
    deps = [
        "//kythe/go/util/log",
        "@com_github_google_go_cmp//cmp",
    ],
)
