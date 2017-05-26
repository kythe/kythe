load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "kindex",
    srcs = ["kindex.go"],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/delimited",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/ptypes",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "kindex_test",
    size = "small",
    srcs = ["kindex_test.go"],
    library = "kindex",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:go_proto_go",
        "//kythe/proto:storage_proto_go",
        "//third_party/proto:any_proto_go",
        "@go_protobuf//:proto",
    ],
)
