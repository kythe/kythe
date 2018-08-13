load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "mavencmd",
    srcs = [
        "mavencmd.go",
        "pom_xml_modifier.go",
    ],
    deps = [
        "//kythe/go/extractors/config/runextractor/backup",
        "//kythe/go/extractors/config/runextractor/constants",
        "//kythe/go/util/cmdutil",
        "@com_github_beevik_etree//:go_default_library",
        "@com_github_google_subcommands//:go_default_library",
    ],
)

go_test(
    name = "modifier_test",
    srcs = ["pom_xml_modifier_test.go"],
    data = glob(["testdata/**"]),
    library = ":mavencmd",
    deps = [
        "//kythe/go/test/testutil",
    ],
)
