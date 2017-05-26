load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "paths",
    srcs = ["paths.go"],
    deps = [
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/xrefs/assemble",
        "//kythe/go/util/reduce",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/proto:common_proto_go",
        "//kythe/proto:internal_proto_go",
        "//kythe/proto:serving_proto_go",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "paths_test",
    size = "small",
    srcs = ["paths_test.go"],
    library = "paths",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
