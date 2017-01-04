load("//tools:build_rules/go.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "indexer",
    srcs = [
        "emit.go",
        "facts.go",
        "indexer.go",
    ],
    deps = [
        "//kythe/go/extractors/govname",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
        "@go_stringset//:stringset",
        "@go_x_tools//:go/gcexportdata",
    ],
)

go_test(
    name = "indexer_test",
    size = "small",
    srcs = ["indexer_test.go"],
    # TODO(fromberger): Build this with a library rule.
    data = [":testdata/foo.a"],
    library = ":indexer",
    deps = ["@go_protobuf//:proto"],
)

load(":testdata/go_indexer_test.bzl", "go_indexer_test")

go_indexer_test(
    name = "filenode_test",
    srcs = ["testdata/basic/filenode.go"],
    import_path = "test/basic",
)

go_indexer_test(
    name = "varref_test",
    srcs = ["testdata/basic/varref.go"],
)

go_indexer_test(
    name = "funcall_test",
    srcs = ["testdata/basic/funcall.go"],
)
