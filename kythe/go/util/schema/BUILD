load("//tools:build_rules/shims.bzl", "go_binary", "go_library", "go_test")

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
    srcs = ["//kythe/data:schema_index.textproto"],
    outs = ["schema_index.go"],
    cmd = " ".join([
        "$(location :mkdata) -package schema",
        "-input '$(location //kythe/data:schema_index.textproto)'",
        "-output '$@'",
    ]),
    tools = [":mkdata"],
    visibility = ["//visibility:private"],
)

# This rule verifies that the checked-in version of indexdata.go matches the
# generated version used by the Bazel rule. It will fail to build if the two
# differ.
#
# To fix it when it fails, run: update_data.sh
genrule(
    name = "schema_index_sync",
    testonly = True,
    srcs = [
        "indexdata.go",
        ":schema_index",
    ],
    outs = ["schema_index_sync.sh"],
    cmd = " ".join([
        "diff -q '$(location :schema_index)' '$(location :indexdata.go)';",
        "echo \"exit $$?\" > $@",
    ]),
    executable = True,
    visibility = ["//visibility:private"],
)

go_binary(
    name = "mkdata",
    srcs = ["mkdata/mkdata.go"],
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/proto:schema_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
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
