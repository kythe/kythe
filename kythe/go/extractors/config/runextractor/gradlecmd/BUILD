load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "gradlecmd",
    srcs = [
        "gradle_build_modifier.go",
        "gradlecmd.go",
    ],
    deps = [
        "//kythe/go/extractors/config/runextractor/backup",
        "//kythe/go/extractors/config/runextractor/constants",
        "//kythe/go/util/cmdutil",
        "@com_github_google_subcommands//:go_default_library",
    ],
)

go_test(
    name = "modifier_test",
    srcs = ["gradle_build_modifier_test.go"],
    data = glob(["testdata/**"]),
    library = ":gradlecmd",
    deps = [
        "//kythe/go/test/testutil",
    ],
)
