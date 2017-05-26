load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "kindex_file_generator",
    srcs = ["kindex_file_generator.go"],
    deps = [
        "//kythe/go/platform/kindex",
        "//kythe/go/util/kytheuri",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)

sh_test(
    name = "kindex_file_generator_test",
    size = "small",
    srcs = [
        "kindex_file_generator_test.sh",
    ],
    data = [
        ":kindex_file_generator",
        "//kythe/cxx/tools:kindex_tool",
        "//kythe/testdata:BUILD",
    ],
)
