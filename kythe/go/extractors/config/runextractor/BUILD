load("//tools:build_rules/shims.bzl", "go_binary", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "runextractor",
    srcs = ["runextractor.go"],
    deps = [
        "//kythe/go/extractors/config/runextractor/gradlecmd",
        "//kythe/go/extractors/config/runextractor/mavencmd",
        "@com_github_google_subcommands//:go_default_library",
    ],
)
