load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "modifier",
    srcs = [
        "build_gradle_modifier.go",
        "pom_xml_modifier.go",
    ],
    importpath = "kythe.io/kythe/go/extractors/config/preprocessor/modifier",
    deps = [
        "@com_github_beevik_etree//:etree",
    ],
)

go_test(
    name = "modifier_test",
    srcs = ["modifier_test.go"],
    data = glob(["testdata/**"]),
    library = ":modifier",
    deps = [
        "//kythe/go/test/testutil",
    ],
)
