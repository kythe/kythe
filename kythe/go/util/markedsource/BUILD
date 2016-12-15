load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "markedsource",
    srcs = ["markedsource.go"],
    deps = [
        "//kythe/proto:xref_proto_go",
    ],
)

go_test(
    name = "markedsource_test",
    srcs = ["markedsource_test.go"],
    library = "markedsource",
    visibility = ["//visibility:private"],
    deps = [
        "@go_protobuf//:proto",
    ],
)
