load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "markedsource",
    srcs = ["markedsource.go"],
    deps = ["//kythe/proto:common_proto_go"],
)

go_test(
    name = "markedsource_test",
    size = "small",
    srcs = ["markedsource_test.go"],
    data = ["//kythe/cxx/doc"],
    library = "markedsource",
    visibility = ["//visibility:private"],
    deps = [
        "@go_protobuf//:proto",
    ],
)
