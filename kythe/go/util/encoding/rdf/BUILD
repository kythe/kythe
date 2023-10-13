load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "rdf",
    srcs = ["rdf.go"],
    importpath = "kythe.io/kythe/go/util/encoding/rdf",
)

go_test(
    name = "rdf_test",
    size = "small",
    srcs = ["rdf_test.go"],
    library = ":rdf",
    visibility = ["//visibility:private"],
)
