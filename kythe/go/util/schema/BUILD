# The checked-in generated files confuse gazelle.
# gazelle:ignore
load("//tools:build_rules/shims.bzl", "go_library", "go_test")
load("@aspect_bazel_lib//lib:write_source_files.bzl", "write_source_file")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "schema",
    srcs = [
        "schema.go",
        "schema_index.go",
    ],
    importpath = "kythe.io/kythe/go/util/schema",
    deps = [
        "//kythe/go/util/schema/facts",
        "//kythe/proto:schema_go_proto",
        "//kythe/proto:storage_go_proto",
    ],
)

genrule(
    name = "schema_index",
    outs = ["schema_index.go"],
    cmd = " ".join([
        "$(location //kythe/go/util/schema/mkdata) --language go --package schema",
        "-output '$@'",
    ]),
    tools = ["//kythe/go/util/schema/mkdata"],
    visibility = ["//visibility:private"],
)

write_source_file(
    name = "schema_index_sync",
    in_file = ":schema_index",
    out_file = "indexdata.go",
)

go_test(
    name = "schema_test",
    size = "small",
    srcs = ["schema_test.go"],
    library = ":schema",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/proto:schema_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
