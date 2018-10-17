load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "modifier",
    srcs = [
        "build_gradle_modifier.go",
        "pom_xml_modifier.go",
    ],
    deps = [
        "@com_github_beevik_etree//:go_default_library",
    ],
)

go_test(
    name = "pom_xml_modifier_test",
    srcs = ["pom_xml_modifier_test.go"],
    data = glob(["testdata/**/*.xml"]),
    library = ":modifier",
    deps = [
        "//kythe/go/test/testutil",
    ],
)

go_test(
    name = "build_gradle_modifier_test",
    srcs = ["build_gradle_modifier_test.go"],
    data = glob(["testdata/**/*build.gradle"]),
    library = ":modifier",
    deps = [
        "//kythe/go/test/testutil",
    ],
)
