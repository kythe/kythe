load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "indexer",
    srcs = [
        "emit.go",
        "facts.go",
        "indexer.go",
        "markedsource.go",
    ],
    deps = [
        "//kythe/go/extractors/govname",
        "//kythe/go/util/metadata",
        "//kythe/go/util/ptypes",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
        "@org_golang_x_tools//go/gcexportdata:go_default_library",
        "@org_golang_x_tools//go/types/typeutil:go_default_library",
    ],
)

go_test(
    name = "indexer_test",
    size = "small",
    srcs = ["indexer_test.go"],
    # TODO(fromberger): Build this with a library rule.
    data = [":testdata/foo.a"],
    library = ":indexer",
    deps = [
        "//kythe/go/test/testutil",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)

load(":testdata/go_indexer_test.bzl", "go_indexer_test")

go_indexer_test(
    name = "syntax_test",
    srcs = ["testdata/syntax.go"],
)

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
    import_path = "test/fun",
)

go_indexer_test(
    name = "functions_test",
    srcs = ["testdata/basic/functions.go"],
    import_path = "test/fun",
)

go_indexer_test(
    name = "package_test",
    srcs = ["testdata/basic/packages.go"],
    import_path = "test/pkg",
)

go_indexer_test(
    name = "vardef_test",
    srcs = ["testdata/basic/vardef.go"],
)

go_indexer_test(
    name = "typespec_test",
    srcs = ["testdata/basic/typespec.go"],
)

go_indexer_test(
    name = "locals_test",
    srcs = ["testdata/basic/locals.go"],
)

go_indexer_test(
    name = "imports_test",
    srcs = ["testdata/basic/imports.go"],
)

go_indexer_test(
    name = "comment_test",
    srcs = ["testdata/basic/comments.go"],
)

go_indexer_test(
    name = "unsafe_test",
    srcs = ["testdata/unsafe.go"],
)

go_indexer_test(
    name = "satisfies_test",
    srcs = ["testdata/basic/satisfies.go"],
)

go_indexer_test(
    name = "structref_test",
    srcs = ["testdata/basic/structref.go"],
)

go_indexer_test(
    name = "code_function_test",
    srcs = ["testdata/code/funcdecl.go"],
    has_marked_source = True,
    import_path = "funcdecl",
)

go_indexer_test(
    name = "code_interface_test",
    srcs = ["testdata/code/interface.go"],
    has_marked_source = True,
    import_path = "test/iface",
    tags = [
        "broken",
        "manual",
    ],
)

go_indexer_test(
    name = "code_var_test",
    srcs = ["testdata/code/pkgvar.go"],
    has_marked_source = True,
    import_path = "pkgvar",
)

go_indexer_test(
    name = "code_struct_test",
    srcs = ["testdata/code/structtype.go"],
    has_marked_source = True,
    import_path = "structtype",
    tags = [
        "broken",
        "manual",
    ],
)

go_indexer_test(
    name = "code_method_test",
    srcs = ["testdata/code/methdecl.go"],
    has_marked_source = True,
    import_path = "methdecl",
)

go_indexer_test(
    name = "override_test",
    srcs = ["testdata/override.go"],
)

go_indexer_test(
    name = "metadata_test",
    srcs = ["testdata/meta.go"],
    data = ["testdata/meta.go.linkage"],
    metadata_suffix = ".linkage",
)

go_indexer_test(
    name = "anchors_test",
    srcs = ["testdata/basic/anchors.go"],
)

go_indexer_test(
    name = "anonymous_test",
    srcs = ["testdata/basic/anonymous.go"],
)

go_indexer_test(
    name = "structinit_test",
    srcs = ["testdata/structinit.go"],
)

go_indexer_test(
    name = "initcall_test",
    srcs = ["testdata/initcall.go"],
)

go_indexer_test(
    name = "packageinit_test",
    srcs = ["testdata/basic/packageinit.go"],
    import_path = "test/pkginit",
)

go_indexer_test(
    name = "types_test",
    srcs = ["testdata/types.go"],
    has_marked_source = True,
)

# load(":testdata/go_indexer_test.bzl", "go_integration_test")
# TODO(#2375): (closed?) requires MarkedSource resolution in pipeline
# go_integration_test(
#     name = "marked_source_integration_test",
#     srcs = ["testdata/integration/marked_source.go"],
#     file_tickets = [
#         "kythe:?path=src/test/marked_source/marked_source.go",
#     ],
#     has_marked_source = True,
# )
