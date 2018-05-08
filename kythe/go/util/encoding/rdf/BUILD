load("//tools:build_rules/shims.bzl", "go_test", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
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
