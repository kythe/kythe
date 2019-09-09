load("//tools:build_rules/shims.bzl", "go_library", "go_test")
load("//tools:build_rules/testing.bzl", "file_diff_test")

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
        "@com_github_golang_protobuf//proto:go_default_library",
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

# This test verifies that the checked-in version of indexdata.go matches the
# generated version used by the Bazel rule. It will fail to build if the two
# differ.
file_diff_test(
    name = "schema_index_test",
    file1 = "indexdata.go",
    file2 = ":schema_index",
    message = "Run update_data.sh to sync indexdata.go with changes",
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
