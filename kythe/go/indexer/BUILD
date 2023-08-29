load("//tools:build_rules/shims.bzl", "go_library", "go_test")
load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load(":testdata/go_indexer_test.bzl", "go_indexer_test")
load("//kythe/go/test/tools/empty_corpus_checker:empty_corpus_test.bzl", "empty_corpus_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "indexer",
    srcs = [
        "emit.go",
        "facts.go",
        "indexer.go",
        "markedsource.go",
    ],
    importpath = "kythe.io/kythe/go/indexer",
    deps = [
        "//kythe/go/extractors/govname",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/go/util/metadata",
        "//kythe/go/util/ptypes",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:metadata_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@org_bitbucket_creachadair_stringset//:stringset",
        "@org_golang_x_tools//go/gcexportdata",
        "@org_golang_x_tools//go/types/typeutil",
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
        "//kythe/go/util/log",
        "//kythe/go/util/metadata",
        "//kythe/go/util/ptypes",
        "//kythe/go/util/schema/edges",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:metadata_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)

bzl_library(
    name = "go_indexer_test_bzl",
    srcs = ["testdata/go_indexer_test.bzl"],
    visibility = ["//visibility:private"],
)

go_indexer_test(
    name = "builtin_test",
    srcs = ["testdata/builtin.go"],
    extra_goals = ["testdata/builtin.go"],
    has_marked_source = True,
    import_path = "builtin",
    resolve_code_facts = True,
    use_file_as_top_level_scope = True,
)

go_indexer_test(
    name = "genericstruct_test",
    srcs = ["testdata/generics/genericstruct.go"],
    has_marked_source = True,
)

go_indexer_test(
    name = "genericmethod_test",
    srcs = ["testdata/generics/genericmethod.go"],
    has_marked_source = True,
)

go_indexer_test(
    name = "genericinterface_test",
    srcs = ["testdata/generics/genericinterface.go"],
    has_marked_source = True,
)

go_indexer_test(
    name = "genericfunc_test",
    srcs = ["testdata/generics/genericfunc.go"],
    has_marked_source = True,
)

go_indexer_test(
    name = "genericdep_test",
    srcs = ["testdata/generics/genericdep.go"],
    has_marked_source = True,
    deps = [":genericinterface_test"],
)

go_indexer_test(
    name = "syntax_test",
    srcs = ["testdata/syntax.go"],
)

go_indexer_test(
    name = "deprecation_test",
    srcs = ["testdata/basic/deprecation.go"],
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
    name = "funcallid_test",
    srcs = ["testdata/basic/funcallid.go"],
    extra_indexer_args = ["-emit_ref_call_over_identifier"],
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
    name = "writes_test",
    srcs = ["testdata/basic/writes.go"],
)

go_indexer_test(
    name = "imports_test",
    srcs = ["testdata/basic/imports.go"],
)

go_indexer_test(
    name = "scopes_test",
    srcs = ["testdata/basic/scopes.go"],
    emit_anchor_scopes = True,
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
    name = "inline_test",
    srcs = ["testdata/basic/inline.go"],
    metadata_suffix = ".linkage",
)

go_indexer_test(
    name = "code_rendered_test",
    srcs = ["testdata/code/rendered.go"],
    has_marked_source = True,
    import_path = "rendered",
    resolve_code_facts = True,
    deps = [":builtin_test"],
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
    name = "anonref_test",
    srcs = ["testdata/basic/anonref.go"],
    deps = [":anonymous_test"],
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
    name = "packageinit_file_test",
    srcs = ["testdata/basic/packageinit_file.go"],
    import_path = "test/pkginit",
    use_file_as_top_level_scope = True,
)

go_indexer_test(
    name = "types_test",
    srcs = ["testdata/types.go"],
    has_marked_source = True,
)

go_indexer_test(
    name = "dep.v2",
    srcs = ["testdata/dep.v2.go"],
)

go_indexer_test(
    name = "dependent_test",
    srcs = ["testdata/dependent.go"],
    has_marked_source = True,
    deps = [
        ":dep.v2",
        ":types_test",
    ],
)

go_indexer_test(
    name = "tappcorpus_test",
    srcs = ["testdata/basic/tappcorpus.go"],
    use_compilation_corpus_for_all = True,
)

empty_corpus_test(
    name = "empty_corpus_test",
    entries = ":tappcorpus_test_entries.entries.gz",
)

go_indexer_test(
    name = "stdlibimport_test",
    srcs = ["testdata/basic/stdlibimport.go"],
    extra_extractor_args = [
        "--use_default_corpus_for_stdlib",
        "--corpus=kythe",
    ],
    use_compilation_corpus_for_all = True,
)

# Test that extracting and indexing a go package that imports standard libraries
# does not contain entries in the "golang.org" corpus when
# --use_default_corpus_for_stdlib is enabled.
empty_corpus_test(
    name = "stdlibimport_corpus_test",
    allowed_corpora = ["kythe"],
    entries = ":stdlibimport_test_entries.entries.gz",
)

go_indexer_test(
    name = "stdliboverride_test",
    srcs = ["testdata/basic/stdliboverride.go"],
    extra_extractor_args = [
        "--corpus=kythe",
    ],
    override_stdlib_corpus = "STDLIB_OVERRIDE",
    use_compilation_corpus_for_all = True,
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
