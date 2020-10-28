load("//tools:build_rules/shims.bzl", "go_library")
load("//tools:build_rules/testing.bzl", "shell_tool_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "metadatacmd",
    srcs = [
        "metadatacmd.go",
    ],
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/go/platform/kzip/buildmetadata",
        "//kythe/go/platform/tools/kzip/flags",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/cmdutil",
        "@com_github_google_subcommands//:go_default_library",
    ],
)

shell_tool_test(
    name = "test",
    scriptfile = "test.sh",
    tools = {
        "KZIP": "//kythe/go/platform/tools/kzip",
    },
)
