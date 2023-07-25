load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "kzip",
    srcs = ["kzip.go"],
    deps = [
        "//kythe/go/platform/tools/kzip/createcmd",
        "//kythe/go/platform/tools/kzip/filtercmd",
        "//kythe/go/platform/tools/kzip/infocmd",
        "//kythe/go/platform/tools/kzip/mergecmd",
        "//kythe/go/platform/tools/kzip/metadatacmd",
        "//kythe/go/platform/tools/kzip/viewcmd",
        "@com_github_google_subcommands//:subcommands",
    ],
)
