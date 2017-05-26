load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "reduce",
    srcs = [
        "reduce.go",
        ":reduce_sort.go",
    ],
    deps = [
        "//kythe/go/util/disksort",
        "//kythe/proto:internal_proto_go",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "reduce_test",
    size = "small",
    srcs = ["reduce_test.go"],
    library = "reduce",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
