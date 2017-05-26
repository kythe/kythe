load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "keyvalue",
    srcs = ["keyvalue.go"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/util/datasize",
        "//kythe/proto:storage_proto_go",
    ],
)

go_test(
    name = "keyvalue_test",
    size = "small",
    srcs = ["keyvalue_test.go"],
    library = "keyvalue",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)
