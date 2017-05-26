load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "rdf",
    srcs = ["rdf.go"],
)

go_test(
    name = "rdf_test",
    size = "small",
    srcs = ["rdf_test.go"],
    library = "rdf",
    visibility = ["//visibility:private"],
)
