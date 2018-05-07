load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "schema",
    srcs = [
        "schema.go",
        ":schema_index",
    ],
    deps = [
        "//kythe/go/util/schema/facts",
        "//kythe/proto:schema_go_proto",
        "//kythe/proto:storage_go_proto",
        "@go_protobuf//:proto",
    ],
)

genrule(
    name = "schema_index",
    srcs = ["//kythe/data:schema_index.textproto"],
    outs = ["schema_index.go"],
    cmd = "{ " + "\n".join([
        "echo 'package schema'",
        "echo -n 'const indexTextPB = `'",
        "cat $(location //kythe/data:schema_index.textproto)",
        "echo '`'",
    ]) + "\n }  > $@",
    visibility = ["//visibility:private"],
)

go_test(
    name = "schema_test",
    size = "small",
    srcs = ["schema_test.go"],
    library = "schema",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/util/schema/nodes",
    ],
)
