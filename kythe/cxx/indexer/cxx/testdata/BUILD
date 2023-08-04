load(
    "//tools/build_rules/verifier_test:cc_indexer_test.bzl",
    "cc_extract_kzip",
    "cc_extractor_test",
    "cc_indexer_test",
    "objc_indexer_test",
)
load("//kythe/go/test/tools/empty_corpus_checker:empty_corpus_test.bzl", "empty_corpus_test")

package(
    default_visibility = ["//kythe:default_visibility"],
    features = ["layering_check"],
)

exports_files(
    ["test_vnames.json"],
    visibility = ["//kythe:default_visibility"],
)

cc_indexer_test(
    name = "static_variable_test",
    srcs = ["basic/static_variable.cc"],
    tags = ["basic"],
)

cc_indexer_test(
    name = "embedded_tag_decl_childof",
    srcs = ["basic/embedded_tag_decl_childof.c"],
    std = "c99",
    tags = ["basic"],
)

cc_indexer_test(
    name = "anonymous_namespace_header",
    srcs = ["basic/anonymous_namespace_header.cc"],
    bundled = True,
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "anonymous_namespace_inc",
    srcs = ["basic/anonymous_namespace_inc.cc"],
    bundled = True,
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "anonymous_namespace_transitive_header",
    srcs = ["basic/anonymous_namespace_transitive_header.cc"],
    bundled = True,
    check_for_singletons = True,
    ignore_dups = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "anonymous_nested_namespace",
    srcs = ["basic/anonymous_nested_namespace.cc"],
    bundled = True,
    check_for_singletons = True,
    ignore_dups = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "anonymous_namespace",
    srcs = ["basic/anonymous_namespace.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "anonymous_inline_namespace",
    srcs = ["basic/anonymous_inline_namespace.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "nested_namespace_definition",
    srcs = ["basic/nested_namespace_definition.cc"],
    check_for_singletons = True,
    std = "c++17",
    tags = [
        "basic",
        "c++17",
    ],
)

cc_indexer_test(
    name = "file_call",
    srcs = ["basic/file_call.cc"],
    check_for_singletons = True,
    convert_marked_source = True,
    std = "c++17",
    tags = [
        "basic",
        "c++17",
    ],
)

cc_indexer_test(
    name = "bundle_self_test",
    srcs = ["bundle_self_test.cc"],
    bundled = True,
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "bundle_self_test_mix",
    srcs = ["bundle_self_test_mix.cc"],
    bundled = True,
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "bundle_self_test_multi_transcript",
    srcs = ["bundle_self_test_multi_transcript.cc"],
    bundled = True,
    check_for_singletons = True,
    ignore_dups = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "bundle_self_test_unclaimed",
    srcs = ["bundle_self_test_unclaimed.cc"],
    bundled = True,
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "bundle_self_test_vnames_json",
    srcs = ["bundle_self_test_vnames_json.cc"],
    bundled = True,
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "claim_macro_features",
    srcs = ["claim_macro_features.cc"],
    bundled = True,
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "include_dotdot",
    srcs = ["include_dotdot.cc"],
    bundled = True,
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "fail_verify",
    srcs = ["test_fail_verify.cc"],
    check_for_singletons = True,
    expect_fail_verify = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "alias_alias_int",
    srcs = ["basic/alias_alias_int.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "alias_alias_ptr_int",
    srcs = ["basic/alias_alias_ptr_int.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "alias_and_cvr",
    srcs = ["basic/alias_and_cvr.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "alias_int",
    srcs = ["basic/alias_int.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "alias_int_twice",
    srcs = ["basic/alias_int_twice.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "alias_root",
    srcs = ["basic/alias_root.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "alias_root_transitive",
    srcs = ["basic/alias_root_transitive.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "alias_root_quals",
    srcs = ["basic/alias_root_quals.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "anchor_utf8",
    srcs = ["basic/anchor_utf8.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "attribute_deprecated",
    srcs = ["basic/attribute_deprecated.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "attribute_availability",
    srcs = ["basic/attribute_availability.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "auto",
    srcs = ["basic/auto.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "auto_const_ref",
    srcs = ["basic/auto_const_ref.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "auto_multi",
    srcs = ["basic/auto_multi.cc"],
    check_for_singletons = True,
    ignore_dups = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "auto_zoo",
    srcs = ["basic/auto_zoo.cc"],
    check_for_singletons = True,
    ignore_dups = True,
    std = "c++14",
    tags = [
        "basic",
        "c++14",
    ],
)

cc_indexer_test(
    name = "enum_fwd_ref",
    srcs = ["basic/enum_fwd_ref.cc"],
    tags = ["basic"],
)

cc_indexer_test(
    name = "builtin_functions",
    srcs = ["basic/builtin_functions.c"],
    check_for_singletons = True,
    copts = ["-Wno-error=implicit-function-declaration"],
    std = "gnu99",
    tags = ["basic"],
)

cc_indexer_test(
    name = "builtin_functions_ctd",
    srcs = ["basic/builtin_functions_ctd.c"],
    check_for_singletons = True,
    ignore_dups = True,
    std = "gnu99",
    tags = ["basic"],
)

cc_indexer_test(
    name = "decltype_auto",
    srcs = ["basic/decltype_auto.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "decltype",
    srcs = ["basic/decltype.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "decltype_const_ref",
    srcs = ["basic/decltype_const_ref.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "decltype_parens",
    srcs = ["basic/decltype_parens.cc"],
    check_for_singletons = True,
    ignore_dups = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "empty_case",
    srcs = ["basic/empty_case.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "enum_class_decl",
    srcs = ["basic/enum_class_decl.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "enum_class_element_decl",
    srcs = ["basic/enum_class_element_decl.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "enum_decl",
    srcs = ["basic/enum_decl.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "enum_decl_completes",
    srcs = ["basic/enum_decl_completes.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "enum_decl_ty",
    srcs = ["basic/enum_decl_ty.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "enum_decl_ty_completes",
    srcs = ["basic/enum_decl_ty_completes.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "enum_decl_ty_header_completes",
    srcs = ["basic/enum_decl_ty_header_completes.cc"],
    check_for_singletons = True,
    tags = ["basic"],
    deps = ["basic/enum_decl_ty_header_completes.h"],
)

cc_indexer_test(
    name = "enum_element_decl",
    srcs = ["basic/enum_element_decl.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "file_content",
    srcs = ["basic/file_content.cc"],
    check_for_singletons = True,
    tags = ["basic"],
    deps = ["basic/a.h"],
)

cc_indexer_test(
    name = "file_node",
    srcs = ["basic/file_node.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "file_node_reentrant",
    srcs = ["basic/file_node_reentrant.cc"],
    check_for_singletons = True,
    tags = ["basic"],
    deps = ["basic/file_node_reentrant.h"],
)

cc_indexer_test(
    name = "inline_namespace",
    srcs = ["basic/inline_namespace.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "macros_builtin",
    srcs = ["basic/macros_builtin.c"],
    check_for_singletons = True,
    std = "c99",
    tags = ["basic"],
)

cc_indexer_test(
    name = "macros_defn",
    srcs = ["basic/macros_defn.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "macros_expand",
    srcs = ["basic/macros_expand.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "macros_expand_transitive",
    srcs = ["basic/macros_expand_transitive.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "macros_if",
    srcs = ["basic/macros_if.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "macros_ifdef",
    srcs = ["basic/macros_ifdef.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "macros_if_defined",
    srcs = ["basic/macros_if_defined.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "macros_implicit",
    srcs = ["basic/macros_implicit.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "macros_include",
    srcs = ["basic/macros_include.cc"],
    check_for_singletons = True,
    tags = ["basic"],
    deps = ["basic/a.h"],
)

cc_indexer_test(
    name = "macros_internal_exp_ref",
    srcs = ["basic/macros_internal_exp_ref.cc"],
    check_for_singletons = True,
    ignore_dups = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "macros_internal_type_ref",
    srcs = ["basic/macros_internal_type_ref.cc"],
    check_for_singletons = True,
    ignore_dups = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "macros_subst_one_level",
    srcs = ["basic/macros_subst_one_level.cc"],
    check_for_singletons = True,
    ignore_dups = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "macros_undef",
    srcs = ["basic/macros_undef.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "namespace",
    srcs = ["basic/namespace.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "namespace_contains",
    srcs = ["basic/namespace_contains.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "namespace_header",
    srcs = ["basic/namespace_header.cc"],
    bundled = True,
    check_for_singletons = True,
    ignore_dups = True,  # namespace foo has the same documentation in two .hs.
    tags = ["basic"],
)

cc_indexer_test(
    name = "namespace_vnames",
    srcs = ["basic/namespace_vnames.cc"],
    bundled = True,
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "nested_name_specifiers",
    srcs = ["basic/nested_name_specifiers.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "new",
    srcs = ["basic/new.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "new_operator",
    srcs = ["basic/new_operator.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "sizeof_ref",
    srcs = ["basic/sizeof_ref.c"],
    check_for_singletons = True,
    std = "gnu99",
    tags = ["basic"],
)

cc_indexer_test(
    name = "sizeof_typedef_ref",
    srcs = ["basic/sizeof_typedef_ref.c"],
    check_for_singletons = True,
    std = "gnu99",
    tags = ["basic"],
)

cc_indexer_test(
    name = "typedef_class",
    srcs = ["basic/typedef_class.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "typedef_class_nested_ns",
    srcs = ["basic/typedef_class_nested_ns.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "typedef_const_int",
    srcs = ["basic/typedef_const_int.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "typedef_int",
    srcs = ["basic/typedef_int.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "typedef_nested_class",
    srcs = ["basic/typedef_nested_class.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "typedef_paren",
    srcs = ["basic/typedef_paren.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "typedef_ptr_int_canonicalized",
    srcs = ["basic/typedef_ptr_int_canonicalized.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "typedef_ptr_int",
    srcs = ["basic/typedef_ptr_int.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "typedef_same",
    srcs = ["basic/typedef_same.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "typeof_param",
    srcs = ["basic/typeof_param.c"],
    check_for_singletons = True,
    ignore_unimplemented = True,
    std = "gnu99",
    tags = ["basic"],
)

cc_indexer_test(
    name = "usr_alias",
    srcs = ["basic/usr_alias.cc"],
    check_for_singletons = True,
    experimental_usr_byte_size = 20,
    ignore_dups = True,
    ignore_unimplemented = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "usr_enum",
    srcs = ["basic/usr_enum.cc"],
    check_for_singletons = True,
    experimental_usr_byte_size = 20,
    ignore_dups = True,
    ignore_unimplemented = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "usr_function",
    srcs = ["basic/usr_function.cc"],
    check_for_singletons = True,
    experimental_usr_byte_size = 20,
    ignore_dups = True,
    ignore_unimplemented = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "usr_macro",
    srcs = ["basic/usr_macro.cc"],
    check_for_singletons = True,
    experimental_usr_byte_size = 20,
    ignore_dups = True,
    ignore_unimplemented = True,
    tags = ["basic"],
)

cc_extract_kzip(
    name = "usr_macro_default_kzip",
    srcs = ["basic/usr_macro_default.cc"],
    corpus = "unit",
    vnames_config = "basic/vname_corpus.json",
)

cc_indexer_test(
    name = "usr_macro_default",
    srcs = [":usr_macro_default_kzip"],
    check_for_singletons = True,
    emit_usr_corpus = True,
    experimental_usr_byte_size = 20,
    ignore_dups = True,
    ignore_unimplemented = True,
    tags = ["basic"],
    use_compilation_corpus_as_default = True,
)

cc_indexer_test(
    name = "usr_record",
    srcs = ["basic/usr_record.cc"],
    check_for_singletons = True,
    experimental_usr_byte_size = 20,
    ignore_dups = True,
    ignore_unimplemented = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "usr_var",
    srcs = ["basic/usr_var.cc"],
    check_for_singletons = True,
    experimental_usr_byte_size = 20,
    ignore_dups = True,
    ignore_unimplemented = True,
    tags = ["basic"],
)

cc_extract_kzip(
    name = "usr_var_default_kzip",
    srcs = ["basic/usr_var_default.cc"],
    corpus = "unit",
    vnames_config = "basic/vname_corpus.json",
)

cc_indexer_test(
    name = "usr_var_default",
    srcs = [":usr_var_default_kzip"],
    check_for_singletons = True,
    emit_usr_corpus = True,
    experimental_usr_byte_size = 20,
    ignore_dups = True,
    ignore_unimplemented = True,
    tags = ["basic"],
    use_compilation_corpus_as_default = True,
)

cc_indexer_test(
    name = "vardecl_double_shadowed_local_anchor",
    srcs = ["basic/vardecl_double_shadowed_local_anchor.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "vardecl_field",
    srcs = ["basic/vardecl_field.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "vardecl_field_ref",
    srcs = ["basic/vardecl_field_ref.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "vardecl_global_anchor",
    srcs = ["basic/vardecl_global_anchor.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "vardecl_global_anon_ns_anchor",
    srcs = ["basic/vardecl_global_anon_ns_anchor.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "vardecl_global_tu_anchor",
    srcs = ["basic/vardecl_global_tu_anchor.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "vardecl_local_anchor",
    srcs = ["basic/vardecl_local_anchor.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "vardecl_shadowed_local_anchor",
    srcs = ["basic/vardecl_shadowed_local_anchor.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "wild_std_allocator",
    srcs = ["basic/wild_std_allocator.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "wild_struct_field",
    srcs = ["basic/wild_struct_field.c"],
    check_for_singletons = True,
    std = "c99",
    tags = ["basic"],
)

cc_indexer_test(
    name = "enum_element_ref",
    srcs = ["basic/enum_element_ref.cc"],
    check_for_singletons = True,
    tags = ["basic"],
)

cc_indexer_test(
    name = "using_declaration_ref",
    srcs = ["basic/using_declaration_ref.cc"],
    tags = ["basic"],
)

cc_indexer_test(
    name = "binary_literals",
    srcs = ["basic/binary_literals.cc"],
    std = "c++14",
    tags = [
        "basic",
        "c++14",
    ],
)

cc_indexer_test(
    name = "generalized_constexpr",
    srcs = ["basic/generalized_constexpr.cc"],
    ignore_dups = True,
    std = "c++14",
    tags = [
        "basic",
        "c++14",
    ],
)

cc_indexer_test(
    name = "constexpr_ref_call",
    srcs = ["basic/constexpr_ref_call.cc"],
    ignore_dups = True,
    std = "c++17",
    tags = [
        "basic",
        "c++17",
    ],
)

cc_indexer_test(
    name = "structured_bindings",
    srcs = ["basic/structured_bindings.cc"],
    check_for_singletons = True,
    ignore_dups = True,
    std = "c++17",
    tags = [
        "basic",
        "c++17",
    ],
)

cc_indexer_test(
    name = "if_switch_init",
    srcs = ["basic/if_switch_init.cc"],
    std = "c++17",
    tags = [
        "basic",
        "c++17",
    ],
)

cc_indexer_test(
    name = "macros_lambda_call",
    srcs = ["basic/macros_lambda_call.cc"],
    ignore_dups = True,
    tags = [
        "basic",
        "manual",  # https://github.com/kythe/kythe/issues/2899
    ],
)

cc_indexer_test(
    name = "build_config",
    srcs = ["basic/build_config.cc"],
    ibuild_config = "test-build-config",
    tags = [
        "basic",
    ],
)

cc_extract_kzip(
    name = "vname_corpus_kzip",
    srcs = ["basic/vname_corpus.cc"],
    corpus = "unit",
    opts = [
        "-DMY_MACRO=1",
        "-UMY_MACRO",
        "-DMY_MACRO=2",
    ],
    vnames_config = "basic/vname_corpus.json",
)

cc_indexer_test(
    name = "vname_corpus",
    srcs = [":vname_corpus_kzip"],
    tags = [
        "basic",
    ],
    use_compilation_corpus_as_default = True,
)

empty_corpus_test(
    name = "empty_corpus_test",
    allowed_corpora = [
        "file",
        "unit",
    ],
    entries = ":vname_corpus_entries.entries.gz",
)

cc_indexer_test(
    name = "macro_body",
    srcs = ["basic/macro_body.cc"],
    ignore_dups = True,
    tags = ["basic"],
)

test_suite(
    name = "indexer_basic",
    tags = ["basic"],
)

cc_indexer_test(
    name = "function_args_decl",
    srcs = ["function/function_args_decl.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_args_defn",
    srcs = ["function/function_args_defn.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_lambda_args",
    srcs = ["function/function_lambda_args.cc"],
    ignore_dups = True,
    tags = ["function"],
)

cc_indexer_test(
    name = "function_nested_rec",
    srcs = ["function/function_nested_rec.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_auto_return",
    srcs = ["function/function_auto_return.cc"],
    std = "c++14",
    tags = [
        "c++14",
        "function",
    ],
)

cc_indexer_test(
    name = "function_call_directness",
    srcs = ["function/function_call_directness.cc"],
    record_call_directness = True,
    tags = ["function"],
)

cc_indexer_test(
    name = "function_ctor",
    srcs = ["function/function_ctor.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_ctor_default_value",
    srcs = ["function/function_ctor_default_value.cc"],
    ignore_dups = True,
    tags = ["function"],
)

cc_indexer_test(
    name = "function_ctor_default_and_init",
    srcs = ["function/function_ctor_default_and_init.cc"],
    ignore_dups = True,
    tags = ["function"],
)

cc_indexer_test(
    name = "function_ctor_init_list",
    srcs = ["function/function_ctor_init_list.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_ctor_base_delegate",
    srcs = ["function/function_ctor_base_delegate.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_ctor_init_refs",
    srcs = ["function/function_ctor_init_refs.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_ctor_new",
    srcs = ["function/function_ctor_new.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_decl",
    srcs = ["function/function_decl.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_decl_completes",
    srcs = ["function/function_decl_completes.cc"],
    tags = ["function"],
    deps = ["function/void_f.h"],
)

cc_indexer_test(
    name = "function_defn_call",
    srcs = ["function/function_defn_call.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_defn",
    srcs = ["function/function_defn.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_defines",
    srcs = ["function/function_defines.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_direct_call",
    srcs = ["function/function_direct_call.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_dtor",
    srcs = ["function/function_dtor.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_dtor_delete",
    srcs = ["function/function_dtor_delete.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_dtor_explicit_call",
    srcs = ["function/function_dtor_explicit_call.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_dtor_implicit_call",
    srcs = ["function/function_dtor_implicit_call.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_knr_ty",
    srcs = ["function/function_knr_ty.c"],
    std = "c99",
    tags = ["function"],
)

cc_indexer_test(
    name = "function_knr_defn",
    srcs = ["function/function_knr_defn.c"],
    std = "gnu89",
    tags = ["function"],
)

cc_indexer_test(
    name = "function_picky_types",
    srcs = ["function/function_picky_types.c"],
    convert_marked_source = True,
    ignore_dups = True,
    std = "gnu89",
    tags = ["function"],
)

cc_indexer_test(
    name = "function_lambda",
    srcs = ["function/function_lambda.cc"],
    ignore_dups = True,
    tags = ["function"],
)

cc_indexer_test(
    name = "function_lambda_value_this",
    srcs = ["function/function_lambda_value_this.cc"],
    ignore_dups = True,
    std = "c++17",
    tags = [
        "c++17",
        "function",
    ],
)

cc_indexer_test(
    name = "function_lambda_explicit_this",
    srcs = ["function/function_lambda_explicit_this.cc"],
    ignore_dups = True,
    tags = [
        "function",
    ],
)

cc_indexer_test(
    name = "function_lambda_implicit_this",
    srcs = ["function/function_lambda_implicit_this.cc"],
    ignore_dups = True,
    tags = [
        "function",
    ],
)

cc_indexer_test(
    name = "function_constexpr_lambda",
    srcs = ["function/function_constexpr_lambda.cc"],
    ignore_dups = True,
    std = "c++17",
    tags = [
        "c++17",
        "function",
    ],
)

cc_indexer_test(
    name = "function_member",
    srcs = ["function/function_member.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_member_conversion",
    srcs = ["function/function_member_conversion.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_member_defines",
    srcs = ["function/function_member_defines.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_member_operator",
    srcs = ["function/function_member_operator.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_operator_overload_dup_names",
    srcs = ["function/function_operator_overload_dup_names.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_operator_overload_enable_if",
    srcs = ["function/function_operator_overload_enable_if.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_operator_overload_names",
    size = "small",
    srcs = ["function/function_operator_overload_names.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_operator_parens_call",
    srcs = ["function/function_operator_parens_call.cc"],
    ignore_dups = True,
    tags = ["function"],
)

cc_indexer_test(
    name = "function_operator_parens_overload_call",
    srcs = ["function/function_operator_parens_overload_call.cc"],
    ignore_dups = True,
    tags = ["function"],
)

cc_indexer_test(
    name = "function_overload_call",
    srcs = ["function/function_overload_call.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_overload",
    srcs = ["function/function_overload.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_ptr_ty",
    srcs = ["function/function_ptr_ty.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_ty",
    srcs = ["function/function_ty.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_static_member",
    srcs = ["function/function_static_member.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_vararg",
    srcs = ["function/function_vararg.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_vararg_ty",
    srcs = ["function/function_vararg_ty.cc"],
    tags = ["function"],
)

cc_indexer_test(
    name = "function_deduced_return",
    srcs = ["function/function_deduced_return.cc"],
    std = "c++14",
    tags = [
        "c++14",
        "function",
    ],
)

cc_indexer_test(
    name = "function_deduced_decltype_return",
    srcs = ["function/function_deduced_decltype_return.cc"],
    # TODO(shahms): decltype(type) return types do not currently work.
    expect_fail_verify = True,
    std = "c++14",
    tags = [
        "c++14",
        "function",
    ],
)

test_suite(
    name = "indexer_function",
    tags = ["function"],
)

cc_indexer_test(
    name = "metadata_inline_cppstyle",
    srcs = ["metadata/metadata_inline_cppstyle.cc"],
    tags = ["metadata"],
)

cc_indexer_test(
    name = "metadata_inline_cstyle",
    srcs = ["metadata/metadata_inline_cstyle.cc"],
    tags = ["metadata"],
)

cc_indexer_test(
    name = "metadata_generates_rule",
    srcs = ["metadata/metadata_generates_rule.cc"],
    tags = ["metadata"],
    deps = ["metadata/single.meta"],
)

cc_indexer_test(
    name = "metadata_generates_rule_defn",
    srcs = ["metadata/metadata_generates_rule_defn.cc"],
    tags = ["metadata"],
    deps = ["metadata/single_defn.meta"],
)

cc_indexer_test(
    name = "metadata_h_generates_rule",
    srcs = ["metadata/metadata_h_generates_rule.cc"],
    tags = ["metadata"],
    deps = ["metadata/single.meta.h"],
)

cc_indexer_test(
    name = "fancy_metadata_h_generates_rule",
    srcs = ["metadata/fancy_metadata_h_generates_rule.cc"],
    tags = ["metadata"],
    deps = ["metadata/fancy_single.meta.h"],
)

cc_indexer_test(
    name = "metadata_pragma",
    srcs = ["metadata/metadata_pragma.cc"],
    tags = ["metadata"],
    deps = ["metadata/empty.meta"],
)

cc_indexer_test(
    name = "metadata_self",
    srcs = ["metadata/metadata_self.cc"],
    # This test attempts to read itself as a metadata file via __FILE__.
    # This is inherently unreliable, but can be improved by this include path.
    copts = ["-I."],
    tags = ["metadata"],
    deps = ["metadata/metadata_self.h"],
)

cc_indexer_test(
    name = "metadata_semantic",
    srcs = ["metadata/metadata_semantic.cc"],
    tags = ["metadata"],
    deps = ["metadata/semantic.meta"],
)

test_suite(
    name = "indexer_metadata",
    tags = ["metadata"],
)

cc_indexer_test(
    name = "docs_bcpl_block",
    srcs = ["docs/docs_bcpl_block.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_bcpl_block_multiline",
    srcs = ["docs/docs_bcpl_block_multiline.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_block",
    srcs = ["docs/docs_block.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_block_multiline",
    srcs = ["docs/docs_block_multiline.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_doc_var",
    srcs = ["docs/docs_doc_var.cc"],
    goal_prefix = "///-",
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_file_level",
    srcs = ["docs/docs_file_level.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_file_level_conflict",
    srcs = ["docs/docs_file_level_conflict.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_file_level_doxygen",
    srcs = ["docs/docs_file_level_doxygen.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_file_level_includes",
    srcs = ["docs/docs_file_level_includes.cc"],
    tags = ["docs"],
    deps = [
        "docs/docs_file_level_includes_a.h",
        "docs/docs_file_level_includes_b.h",
    ],
)

cc_indexer_test(
    name = "docs_fwd",
    srcs = ["docs/docs_fwd.cc"],
    experimental_drop_cpp_fwd_decl_docs = True,
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_implicit_block",
    srcs = ["docs/docs_implicit_block.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_implicit_block_multiline",
    srcs = ["docs/docs_implicit_block_multiline.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_implicit_unrelated_block",
    srcs = ["docs/docs_implicit_unrelated_block.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_implicit_unrelated_block_decl",
    srcs = ["docs/docs_implicit_unrelated_block_decl.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_implicit_unrelated_block_newline",
    srcs = ["docs/docs_implicit_unrelated_block_newline.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_juxtaposed",
    srcs = ["docs/docs_juxtaposed.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_juxtaposed_implicit",
    srcs = ["docs/docs_juxtaposed_implicit.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_juxtaposed_implicit_multiline",
    srcs = ["docs/docs_juxtaposed_implicit_multiline.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_juxtaposed_multiline",
    srcs = ["docs/docs_juxtaposed_multiline.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_linkage_spec",
    srcs = ["docs/docs_linkage_spec.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_ref_gen_backtick",
    srcs = ["docs/docs_ref_gen_backtick.cc"],
    goal_prefix = "///-",
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_ref_gen_doxycommand",
    srcs = ["docs/docs_ref_gen_doxycommand.cc"],
    goal_prefix = "///-",
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_ref_gen_param",
    srcs = ["docs/docs_ref_gen_param.cc"],
    goal_prefix = "///-",
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_ref_gen_quote",
    srcs = ["docs/docs_ref_gen_quote.cc"],
    goal_prefix = "///-",
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_ref_gen_tparam",
    srcs = ["docs/docs_ref_gen_tparam.cc"],
    goal_prefix = "///-",
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_template_alias",
    srcs = ["docs/docs_template_alias.cc"],
    ignore_dups = True,
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_template_function",
    srcs = ["docs/docs_template_function.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_template_record",
    srcs = ["docs/docs_template_record.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_template_var",
    srcs = ["docs/docs_template_var.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_template_partial_spec_record",
    srcs = ["docs/docs_template_partial_spec_record.cc"],
    ignore_dups = True,
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_template_partial_spec_var",
    srcs = ["docs/docs_template_partial_spec_var.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_template_function_spec",
    srcs = ["docs/docs_template_function_spec.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_template_total_spec_record",
    srcs = ["docs/docs_template_total_spec_record.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_template_total_spec_var",
    srcs = ["docs/docs_template_total_spec_var.cc"],
    tags = ["docs"],
)

cc_indexer_test(
    name = "docs_visibility",
    srcs = ["docs/docs_visibility.cc"],
    tags = ["docs"],
)

test_suite(
    name = "indexer_docs",
    tags = ["docs"],
)

cc_indexer_test(
    name = "flags_bool",
    srcs = ["libraries/flags_bool.cc"],
    ignore_dups = True,
    tags = ["libraries"],
    deps = ["libraries/flags_bool.h"] + glob(["libraries/gflags*.h"]),
)

cc_indexer_test(
    name = "flags_bool_stdbool",
    srcs = ["libraries/flags_bool_stdbool.cc"],
    ignore_dups = True,
    tags = ["libraries"],
    deps = ["libraries/stdbool.h"] + glob(["libraries/gflags*.h"]),
)

cc_indexer_test(
    name = "flags_decls",
    srcs = ["libraries/flags_decls.cc"],
    ignore_dups = True,
    tags = ["libraries"],
    deps = glob(["libraries/gflags*.h"]),
)

cc_indexer_test(
    name = "flags_defns",
    srcs = ["libraries/flags_defns.cc"],
    ignore_dups = True,
    tags = ["libraries"],
    deps = glob(["libraries/gflags*.h"]),
)

cc_indexer_test(
    name = "flags_empty",
    srcs = ["libraries/flags_empty.cc"],
    tags = ["libraries"],
    deps = glob(["libraries/gflags*.h"]),
)

cc_indexer_test(
    name = "flags_ref_int64_decl",
    srcs = ["libraries/flags_ref_int64_decl.cc"],
    tags = ["libraries"],
    deps = glob(["libraries/gflags*.h"]),
)

cc_indexer_test(
    name = "flags_ref_int64_defn",
    srcs = ["libraries/flags_ref_int64_defn.cc"],
    ignore_dups = True,
    tags = ["libraries"],
    deps = glob(["libraries/gflags*.h"]),
)

cc_indexer_test(
    name = "flags_ref_int64_defn_completes",
    srcs = ["libraries/flags_ref_int64_defn_completes.cc"],
    ignore_dups = True,
    tags = ["libraries"],
    deps = glob(["libraries/gflags*.h"]),
)

cc_indexer_test(
    name = "flags_string",
    srcs = ["libraries/flags_string.cc"],
    ignore_dups = True,
    tags = ["libraries"],
    deps = ["libraries/flags_string.h"] + glob(["libraries/gflags*.h"]),
)

cc_indexer_test(
    name = "proto_parsetextproto",
    srcs = ["libraries/proto_parsetextproto.cc"],
    ignore_dups = True,
    tags = ["libraries"],
    deps = [
        "libraries/message.proto.h",
        "libraries/parsetextproto.h",
    ],
)

cc_indexer_test(
    name = "proto_parsetextproto_tabs",
    srcs = ["libraries/proto_parsetextproto_tabs.cc"],
    ignore_dups = True,
    tags = ["libraries"],
    deps = [
        "libraries/message.proto.h",
        "libraries/parsetextproto.h",
    ],
)

cc_indexer_test(
    name = "make_unique_factories",
    srcs = ["libraries/make_unique_factories.cc"],
    ignore_dups = True,
    tags = ["libraries"],
)

# Note: this depends on the platform <memory> header and
# as such is a cc_extractor_test rather than hermetic cc_indexer_test.
cc_extractor_test(
    name = "make_shared_factories",
    srcs = ["libraries/make_shared_factories.cc"],
    tags = ["libraries"],
)

test_suite(
    name = "indexer_libraries",
    tags = ["libraries"],
)

cc_indexer_test(
    name = "rec_agg_init",
    srcs = ["rec/rec_agg_init.cc"],
    std = "c++17",
    tags = [
        "c++17",
        "rec",
    ],
)

cc_indexer_test(
    name = "rec_ambiguous_struct",
    srcs = ["rec/rec_ambiguous_struct.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_anon_struct",
    srcs = ["rec/rec_anon_struct.cc"],
    ignore_dups = True,
    tags = ["rec"],
)

cc_indexer_test(
    name = "gnu_anon_struct",
    srcs = ["rec/rec_gnu_anon_struct.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_anon_union",
    srcs = ["rec/rec_anon_union.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_class",
    srcs = ["rec/rec_class.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_class_visibility",
    srcs = ["rec/rec_class_visibility.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_class_base",
    srcs = ["rec/rec_class_base.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_class_base_dependent",
    srcs = ["rec/rec_class_base_dependent.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_class_base_recurring",
    srcs = ["rec/rec_class_base_recurring.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_class_header_completes",
    srcs = ["rec/rec_class_header_completes.cc"],
    tags = ["rec"],
    deps = ["rec/rec_class_header_completes.h"],
)

cc_indexer_test(
    name = "rec_class_macro",
    srcs = ["rec/rec_class_macro.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_ctor_implicit_refs",
    srcs = ["rec/rec_ctor_implicit_refs.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_designated_initializer",
    srcs = ["rec/rec_designated_initializer.c"],
    std = "c99",
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_fwd_struct_name",
    srcs = ["rec/rec_fwd_struct_name.c"],
    std = "c99",
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_fwd_class_ref",
    srcs = ["rec/rec_fwd_class_ref.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_fwd_class_type_ref",
    srcs = ["rec/rec_fwd_class_type_ref.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_inner_class",
    srcs = ["rec/rec_inner_class.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_implicit_anchors",
    srcs = ["rec/rec_implicit_anchors.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_implicit_anchors_ctor",
    srcs = ["rec/rec_implicit_anchors_ctor.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_implicit_ctor_calls",
    srcs = ["rec/rec_implicit_ctor_calls.cc"],
    check_for_singletons = True,
    ignore_dups = True,
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_implicit_ctor_calls_extra",
    srcs = ["rec/rec_implicit_ctor_calls_extra.cc"],
    check_for_singletons = True,
    ignore_dups = True,
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_implicit_ctor_calls_conversion",
    srcs = ["rec/rec_implicit_ctor_calls_conversion.cc"],
    check_for_singletons = True,
    ignore_dups = True,
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_implicit_new",
    srcs = ["rec/rec_implicit_new.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_nested_name",
    srcs = ["rec/rec_nested_name.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_override",
    srcs = ["rec/rec_override.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_override_root",
    srcs = ["rec/rec_override_root.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_struct_c99",
    srcs = ["rec/rec_struct.c"],
    std = "c99",
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_struct",
    srcs = ["rec/rec_struct.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_struct_decl",
    srcs = ["rec/rec_struct_decl.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_union",
    srcs = ["rec/rec_union.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_class_member_defn_call",
    srcs = ["rec/rec_class_member_defn_call.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_class_member_base_call",
    srcs = ["rec/rec_class_member_base_call.cc"],
    check_for_singletons = True,
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_nested_ctor",
    srcs = ["rec/rec_nested_ctor.cc"],
    check_for_singletons = True,
    ignore_dups = True,
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_class_member_ptr",
    srcs = ["rec/rec_class_member_ptr.cc"],
    check_for_singletons = True,
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_ctor_dtor_ref_class",
    srcs = ["rec/rec_ctor_dtor_ref_class.cc"],
    tags = ["rec"],
)

cc_indexer_test(
    name = "rec_write_designated_initializer",
    srcs = ["rec/rec_write_designated_initializer.cc"],
    tags = ["rec"],
)

test_suite(
    name = "indexer_rec",
    tags = ["rec"],
)

# implies the effects of experimental_alias_template_instantiations = True,
# leading to more duplicate ("aliased") output.

cc_indexer_test(
    name = "template_alias_tvar",
    srcs = ["template/template_alias_tvar.cc"],
    check_for_singletons = True,
    experimental_alias_template_instantiations = True,
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_alias_tvar_class",
    srcs = ["template/template_alias_tvar_class.cc"],
    check_for_singletons = True,
    experimental_alias_template_instantiations = True,
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_alias_tvar_alias",
    srcs = ["template/template_alias_tvar_alias.cc"],
    check_for_singletons = True,
    experimental_alias_template_instantiations = True,
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_alias",
    srcs = ["template/template_alias.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_decl_arg",
    srcs = ["template/template_decl_arg.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_alias_naming",
    srcs = ["template/template_alias_naming.cc"],
    check_for_singletons = True,
    experimental_alias_template_instantiations = True,
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_type_alias_param",
    srcs = ["template/template_type_alias_param.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_alias_implicit_instantiation",
    srcs = ["template/template_alias_implicit_instantiation.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_alias_implicit_instantiation_decls",
    srcs = ["template/template_alias_implicit_instantiation_decls.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_alias_multilevel_abs",
    srcs = ["template/template_alias_multilevel_abs.cc"],
    check_for_singletons = True,
    experimental_alias_template_instantiations = True,
    ignore_dups = True,
    ignore_unimplemented = True,  # hash of SubstTemplateTemplateParm
    tags = ["template"],
)

cc_indexer_test(
    name = "template_arg_multiple_typename",
    srcs = ["template/template_arg_multiple_typename.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_arg_typename",
    srcs = ["template/template_arg_typename.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_blame_member",
    srcs = ["template/template_blame_member.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_blame_spec",
    srcs = ["template/template_blame_spec.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_blame_pspec_member",
    srcs = ["template/template_blame_pspec_member.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_blame_spec_member",
    srcs = ["template/template_blame_spec_member.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_class_template_ctor",
    srcs = ["template/template_class_template_ctor.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_class_ctor_default_and_init",
    srcs = ["template/template_class_ctor_default_and_init.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_class_defn",
    srcs = ["template/template_class_defn.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_class_fn_spec",
    srcs = ["template/template_class_fn_spec.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_class_inner",
    srcs = ["template/template_class_inner.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_class_inst_explicit",
    srcs = ["template/template_class_inst_explicit.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_class_inst_implicit",
    srcs = ["template/template_class_inst_implicit.cc"],
    # Type alias alias inside a class template gets the same documentation text.
    # TODO(zarko): Should the two aliases be distinctly named?
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_class_inst_implicit_dependent",
    srcs = ["template/template_class_inst_implicit_dependent.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_class_inst_vs_spec",
    srcs = ["template/template_class_inst_vs_spec.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_class_ref",
    srcs = ["template/template_class_ref.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_class_ref_ps",
    srcs = ["template/template_class_ref_ps.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_class_skip_implicit_on",
    srcs = ["template/template_class_skip_implicit_on.cc"],
    ignore_dups = True,
    index_template_instantiations = False,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_depbase_field_ref",
    srcs = ["template/template_depbase_field_ref.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_dependent_ctor",
    srcs = ["template/template_dependent_ctor.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_dependent_ctor_small",
    srcs = ["template/template_dependent_ctor_small.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_dependent_dtor",
    srcs = ["template/template_dependent_dtor.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_dependent_dtor_qualified",
    srcs = ["template/template_dependent_dtor_qualified.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template/template_dependent_name_ref",
    srcs = ["template/template_dependent_name_ref.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_dependent_sized_array",
    srcs = ["template/template_dependent_sized_array.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_depexpr_field_ref",
    srcs = ["template/template_depexpr_field_ref.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_depname_field_ref",
    srcs = ["template/template_depname_field_ref.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_depname_template_field_ref",
    srcs = ["template/template_depname_template_field_ref.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_depname_class",
    srcs = ["template/template_depname_class.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_depname_implicit",
    srcs = ["template/template_depname_implicit.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_depname_inst_class",
    srcs = ["template/template_depname_inst_class.cc"],
    ignore_dups = True,
    ignore_unimplemented = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_depname_path_graph",
    srcs = ["template/template_depname_path_graph.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_drop_redundant_wraiths_off",
    srcs = ["template/template_drop_redundant_wraiths.cc"],
    experimental_drop_instantiation_independent_data = False,
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_drop_redundant_wraiths_on",
    srcs = ["template/template_drop_redundant_wraiths.cc"],
    experimental_drop_instantiation_independent_data = True,
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_drop_redundant_wraiths_off_expect_fail",
    srcs = ["template/template_drop_redundant_wraiths_expect_fail.cc"],
    experimental_drop_instantiation_independent_data = False,
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_drop_redundant_wraiths_on_expect_fail",
    srcs = ["template/template_drop_redundant_wraiths_expect_fail.cc"],
    expect_fail_verify = True,
    experimental_drop_instantiation_independent_data = True,
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_exclude",
    srcs = ["template/template_exclude.cc"],
    check_for_singletons = True,
    ignore_dups = True,
    tags = ["template"],
    template_instance_exclude_path_pattern = ".*/exclude.*",
    deps = [
        "template/exclude_this_file.h",
        "template/include_this_file.h",
    ],
)

cc_indexer_test(
    name = "template_field",
    srcs = ["template/template_field.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_file_call",
    srcs = ["template/template_file_call.cc"],
    check_for_singletons = True,
    convert_marked_source = True,
    ignore_dups = True,
    std = "c++17",
    tags = [
        "c++17",
        "template",
    ],
)

cc_indexer_test(
    name = "template_fn_call",
    srcs = ["template/template_fn_call.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_decl",
    srcs = ["template/template_fn_decl.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_decl_defn",
    srcs = ["template/template_fn_decl_defn.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_defines",
    srcs = ["template/template_fn_defines.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_defn",
    srcs = ["template/template_fn_defn.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_dependent_spec_defn",
    srcs = ["template/template_fn_dependent_spec_defn.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_explicit_spec_completes",
    srcs = ["template/template_fn_explicit_spec_completes.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_explicit_spec_with_default_completes",
    srcs = ["template/template_fn_explicit_spec_with_default_completes.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_implicit_spec",
    srcs = ["template/template_fn_implicit_spec.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_inst_vs_spec",
    srcs = ["template/template_fn_inst_vs_spec.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_member_spec_defn",
    srcs = ["template/template_fn_member_spec_defn.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_multi_decl_def",
    srcs = ["template/template_fn_multi_decl_def.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_multiple_implicit_spec",
    srcs = ["template/template_fn_multiple_implicit_spec.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_overload",
    srcs = ["template/template_fn_overload.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_spec",
    srcs = ["template/template_fn_spec.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_spec_decl",
    srcs = ["template/template_fn_spec_decl.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_spec_defn_decl",
    srcs = ["template/template_fn_spec_defn_decl.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_spec_defn_defn_decl_decl",
    srcs = ["template/template_fn_spec_defn_defn_decl_decl.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_fn_spec_overload",
    srcs = ["template/template_fn_spec_overload.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_incomplete_array",
    srcs = ["template/template_incomplete_array.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_instance_type_from_class",
    srcs = ["template/template_instance_type_from_class.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_nested_name",
    srcs = ["template/template_nested_name.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_multilevel_argument",
    srcs = ["template/template_multilevel_argument.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_pack_fn_decl",
    srcs = ["template/template_pack_fn_decl.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_pack_fn_mono_call",
    srcs = ["template/template_pack_fn_mono_call.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_pack_fn_pack_call",
    srcs = ["template/template_pack_fn_pack_call.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_pack_rec_pack_call",
    srcs = ["template/template_pack_rec_pack_call.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_ps_completes",
    srcs = ["template/template_ps_completes.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_ps_decl",
    srcs = ["template/template_ps_decl.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_ps_defn",
    srcs = ["template/template_ps_defn.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_ps_multiple_decl",
    srcs = ["template/template_ps_multiple_decl.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_ps_twovar_decl",
    srcs = ["template/template_ps_twovar_decl.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_rec_member_abs",
    srcs = ["template/template_rec_member_abs.cc"],
    experimental_alias_template_instantiations = True,
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_rec_inherit",
    srcs = ["template/template_rec_inherit.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_rec_override",
    srcs = ["template/template_rec_override.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_rec_override_root",
    srcs = ["template/template_rec_override_root.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_rec_override_root_aliasing",
    srcs = ["template/template_rec_override_root_aliasing.cc"],
    check_for_singletons = True,
    experimental_alias_template_instantiations = True,
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_redundant_constituents_anchored",
    srcs = ["template/template_redundant_constituents_anchored.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_static_member",
    srcs = ["template/template_static_member.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_two_arg_spec",
    srcs = ["template/template_two_arg_spec.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_ty_typename",
    srcs = ["template/template_ty_typename.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_unresolved_ctor_expr",
    srcs = ["template/template_unresolved_ctor_expr.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_var_decl",
    srcs = ["template/template_var_decl.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_var_defn",
    srcs = ["template/template_var_defn.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_var_defn_completes",
    srcs = ["template/template_var_defn_completes.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_var_explicit_spec",
    srcs = ["template/template_var_explicit_spec.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_var_implicit_spec",
    srcs = ["template/template_var_implicit_spec.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_var_inst_total_spec",
    srcs = ["template/template_var_inst_total_spec.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_var_ps",
    srcs = ["template/template_var_ps.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_var_ps_completes",
    srcs = ["template/template_var_ps_completes.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_var_ps_implicit_spec",
    srcs = ["template/template_var_ps_implicit_spec.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_var_ref_ps",
    srcs = ["template/template_var_ref_ps.cc"],
    ignore_dups = True,
    tags = ["template"],
)

cc_indexer_test(
    name = "template_var_constexpr",
    srcs = ["template/template_var_constexpr.cc"],
    ignore_dups = True,
    std = "c++14",
    tags = ["template"],
)

cc_indexer_test(
    name = "generic_lambda",
    srcs = ["template/generic_lambda.cc"],
    ignore_dups = True,
    std = "c++14",
    tags = ["template, c++14"],
)

cc_indexer_test(
    name = "template_using_pack_expansion",
    srcs = ["template/template_using_pack_expansion.cc"],
    ignore_dups = True,
    std = "c++17",
    tags = [
        "c++17",
        "template",
    ],
)

cc_indexer_test(
    name = "template_auto_parameter",
    srcs = ["template/template_auto_parameter.cc"],
    ignore_dups = True,
    std = "c++17",
    tags = [
        "c++17",
        "template",
    ],
)

cc_indexer_test(
    name = "template_fold_expressions",
    srcs = ["template/template_fold_expressions.cc"],
    ignore_dups = True,
    std = "c++17",
    tags = [
        "c++17",
        "template",
    ],
)

cc_indexer_test(
    name = "template_class_ctor_deduction",
    srcs = ["template/template_class_ctor_deduction.cc"],
    ignore_dups = True,
    ignore_unimplemented = True,
    std = "c++17",
    tags = [
        "c++17",
        "template",
    ],
)

test_suite(
    name = "indexer_template",
    tags = ["template"],
)

cc_indexer_test(
    name = "ms_tvar",
    srcs = ["marked_source/ms_tvar.cc"],
    convert_marked_source = True,
    std = "c++14",
    tags = ["marked_source"],
)

cc_indexer_test(
    name = "ms_array",
    srcs = ["marked_source/ms_array.cc"],
    convert_marked_source = True,
    std = "c++14",
    tags = ["marked_source"],
)

cc_indexer_test(
    name = "ms_class",
    srcs = ["marked_source/ms_class.cc"],
    convert_marked_source = True,
    std = "c++14",
    tags = ["marked_source"],
)

cc_indexer_test(
    name = "ms_class_ns",
    srcs = ["marked_source/ms_class_ns.cc"],
    convert_marked_source = True,
    std = "c++14",
    tags = ["marked_source"],
)

cc_indexer_test(
    name = "ms_fn",
    srcs = ["marked_source/ms_fn.cc"],
    convert_marked_source = True,
    std = "c++14",
    tags = ["marked_source"],
)

cc_indexer_test(
    name = "ms_macro_fn",
    srcs = ["marked_source/ms_macro_fn.cc"],
    convert_marked_source = True,
    std = "c++14",
    tags = ["marked_source"],
)

cc_indexer_test(
    name = "ms_fn_namespace",
    srcs = ["marked_source/ms_fn_namespace.cc"],
    convert_marked_source = True,
    std = "c++14",
    tags = ["marked_source"],
)

cc_indexer_test(
    name = "ms_fn_rec",
    srcs = ["marked_source/ms_fn_rec.cc"],
    convert_marked_source = True,
    std = "c++14",
    tags = ["marked_source"],
)

cc_indexer_test(
    name = "ms_fn_template",
    srcs = ["marked_source/ms_fn_template.cc"],
    convert_marked_source = True,
    std = "c++14",
    tags = ["marked_source"],
)

cc_indexer_test(
    name = "ms_fn_rec_template",
    srcs = ["marked_source/ms_fn_rec_template.cc"],
    convert_marked_source = True,
    std = "c++14",
    tags = ["marked_source"],
)

cc_indexer_test(
    name = "ms_var_fn_template",
    srcs = ["marked_source/ms_var_fn_template.cc"],
    convert_marked_source = True,
    std = "c++14",
    tags = ["marked_source"],
)

cc_indexer_test(
    name = "ms_fn_ptr_return",
    srcs = ["marked_source/ms_fn_ptr_return.cc"],
    convert_marked_source = True,
    std = "c++14",
    tags = ["marked_source"],
)

objc_indexer_test(
    name = "objc_marked_source_category",
    srcs = ["objc/categories/marked_category.m"],
    check_for_singletons = True,
    convert_marked_source = True,
    tags = [
        "marked_source",
        "objc",
    ],
)

objc_indexer_test(
    name = "objc_marked_source_class",
    srcs = ["objc/marked_class.m"],
    convert_marked_source = True,
    tags = [
        "marked_source",
        "objc",
    ],
)

objc_indexer_test(
    name = "objc_marked_source_method",
    srcs = ["objc/marked_method.m"],
    check_for_singletons = True,
    convert_marked_source = True,
    tags = [
        "marked_source",
        "objc",
    ],
)

objc_indexer_test(
    name = "objc_marked_source_property",
    srcs = ["objc/marked_property.m"],
    check_for_singletons = True,
    convert_marked_source = True,
    ignore_dups = True,
    tags = [
        "marked_source",
        "objc",
    ],
)

objc_indexer_test(
    name = "objc_marked_source_attr__Nullable",
    srcs = ["objc/marked_attr__Nullable.m"],
    convert_marked_source = True,
    tags = [
        "marked_source",
        "objc",
    ],
)

test_suite(
    name = "indexer_marked_source",
    tags = ["marked_source"],
)

# Objective C Tests

objc_indexer_test(
    name = "attribute_nullable",
    srcs = ["objc/attribute_nullable.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "block_pointer",
    srcs = ["objc/block_pointer.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "block_pointer_typedef",
    srcs = ["objc/block_pointer_typedef.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "block_pointer_type",
    srcs = ["objc/block_pointer_type.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "boxed_expr",
    srcs = ["objc/boxed_expr.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "builtin_defs",
    srcs = ["objc/builtin_defs.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "builtin_bool",
    srcs = ["objc/builtin_bool.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "category",
    srcs = ["objc/categories/base+foo.m"],
    ignore_dups = True,
    tags = ["objc"],
    deps = [
        "objc/categories/base.h",
        "objc/categories/base+foo.h",
    ],
)

objc_indexer_test(
    name = "class_decl_defn",
    srcs = ["objc/class_decl_defn.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "class_docs",
    srcs = ["objc/docs/class_docs.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "class_instantiation",
    srcs = ["objc/class_instantiation.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "class_method_call",
    srcs = ["objc/class_method_call.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "class_method_call_variable_params",
    srcs = ["objc/class_method_call_variable_params.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "class_method_decl_defn",
    srcs = ["objc/class_method_decl_defn.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "compatibility_alias",
    srcs = ["objc/compatibility_alias.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "compatibility_alias_and_typedef",
    srcs = ["objc/compatibility_alias_and_typedef.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "compatibility_alias_twice",
    srcs = ["objc/compatibility_alias_twice.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "compatibility_alias_with_macro",
    srcs = ["objc/compatibility_alias_with_macro.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "compatibility_alias_with_macro_with_params",
    srcs = ["objc/compatibility_alias_with_macro_with_params.m"],
    expect_fail_verify = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "encode",
    srcs = ["objc/encode.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "extends",
    srcs = ["objc/extends.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "extends_no_super_impl",
    srcs = ["objc/extends_no_super_impl.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "extension",
    srcs = ["objc/categories/extension.m"],
    tags = ["objc"],
    deps = ["objc/categories/extension.h"],
)

objc_indexer_test(
    name = "extension_property",
    srcs = ["objc/categories/extension_property.m"],
    ignore_dups = True,
    tags = ["objc"],
    deps = ["objc/categories/extension_property.h"],
)

objc_indexer_test(
    name = "forward_declare",
    srcs = ["objc/forward_declare.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "forward_declare_completes",
    srcs = ["objc/forward_declare_completes.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "forward_declare_not_necessary",
    srcs = ["objc/forward_declare_not_necessary.m"],
    tags = ["objc"],
    deps = ["objc/FwdClass.h"],
)

objc_indexer_test(
    name = "forward_declare_not_necessary_2",
    srcs = ["objc/forward_declare_not_necessary_2.m"],
    tags = ["objc"],
    deps = ["objc/FwdClass.h"],
)

objc_indexer_test(
    name = "fwd_decl_docs",
    srcs = ["objc/docs/fwd_decl_docs.m"],
    experimental_drop_objc_fwd_class_docs = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "generic_bound",
    srcs = ["objc/generics/generic_bound.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "generic_contravariant_decl",
    srcs = ["objc/generics/generic_contravariant_decl.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "generic_contravariant_restricted_super_class",
    srcs = ["objc/generics/generic_contravariant_restricted_super_class.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "generic_covariant_decl",
    srcs = ["objc/generics/generic_covariant_decl.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "generic_covariant_restricted_super_class",
    srcs = ["objc/generics/generic_covariant_restricted_super_class.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "generic_inherited",
    srcs = ["objc/generics/generic_inherited.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "generic_invariant",
    srcs = ["objc/generics/generic_invariant.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "generic_instantiation",
    srcs = ["objc/generics/generic_instantiation.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "generic_method_call",
    srcs = ["objc/generics/generic_method_call.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "generic_method_type",
    srcs = ["objc/generics/generic_method_type.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "generic_nested",
    srcs = ["objc/generics/generic_nested.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "generic_restricted_protocol",
    srcs = ["objc/generics/generic_restricted_protocol.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "generic_two_args",
    srcs = ["objc/generics/generic_two_args.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "generic_unbound",
    srcs = ["objc/generics/generic_unbound.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "id_instance_method",
    srcs = ["objc/id_instance_method.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "include",
    srcs = ["objc/include.m"],
    tags = ["objc"],
    deps = ["objc/include.h"],
)

objc_indexer_test(
    name = "instance_method_call",
    srcs = ["objc/instance_method_call.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "instance_method_call_variable_params",
    srcs = ["objc/instance_method_call_variable_params.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "instance_method_call_with_class_params",
    srcs = ["objc/instance_method_call_with_class_params.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "instance_method_call_with_params",
    srcs = ["objc/instance_method_call_with_params.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "instance_method_decl_defn",
    srcs = ["objc/instance_method_decl_defn.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "instance_method_decl_defn_with_params",
    srcs = ["objc/instance_method_decl_defn_with_params.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "instance_method_matched_correctly",
    srcs = ["objc/instance_method_matched_correctly.m"],
    tags = ["objc"],
)

# TODO(salguarnieri) We need to make anchors for instance self and class self.
# At the same time, we should make an anchor for this in c++.
objc_indexer_test(
    name = "instance_method_self_and_class_method_self_different",
    srcs = ["objc/instance_method_self_and_class_method_self_different.m"],
    expect_fail_verify = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "instance_method_type",
    srcs = ["objc/instance_method_type.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "ivar_decl",
    srcs = ["objc/ivar_decl.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "lang_name",
    srcs = ["objc/lang_name.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "method_overrides",
    srcs = ["objc/method_overrides.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "method_overrides_decl_only",
    srcs = ["objc/method_overrides_decl_only.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "method_overrides_root",
    srcs = ["objc/method_overrides_root.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "method_param_types",
    srcs = ["objc/method_param_types.m"],
    tags = ["objc"],
)

# TODO(salguarnieri) Try to implement support for method swizzling. It is
# possible that this will be impossible the handle statically, in which case
# this test should encode the expected behavior of the indexer in this case.
# TODO(salguarnieri) support including <objc/runtime.h> so this test will at
# least make it through the indexer.
objc_indexer_test(
    name = "method_swizzle",
    srcs = ["objc/method_swizzle.m"],
    tags = [
        "arc-ignore",
        "manual",
        "objc",
    ],
)

objc_indexer_test(
    name = "method_type",
    srcs = ["objc/method_type.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "multiline_method",
    srcs = ["objc/multiline_method.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "precise_extends",
    srcs = ["objc/precise_extends.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "property_custom_getter_setter",
    srcs = ["objc/property_custom_getter_setter.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "property_decl_defn",
    srcs = ["objc/property_decl_defn.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "property_decl_defn_class",
    srcs = ["objc/property_decl_defn_class.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "property_decl_defn_synth",
    srcs = ["objc/property_decl_defn_synth.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "property_getter_setter",
    srcs = ["objc/property_getter_setter.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "protocol_class_namespace",
    srcs = ["objc/protocol_class_namespace.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "protocol_defn",
    srcs = ["objc/protocol_defn.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "protocol_extends",
    srcs = ["objc/protocol_extends.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "protocol_extends_impl",
    srcs = ["objc/protocol_extends_impl.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "protocol_impl",
    srcs = ["objc/protocol_impl.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "protocol_method_defn",
    srcs = ["objc/protocol_method_defn.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "protocol_param",
    srcs = ["objc/protocol_param.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "protocol_param_no_id",
    srcs = ["objc/protocol_param_no_id.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "protocol_param_long_enough_to_hash",
    srcs = ["objc/protocol_param_long_enough_to_hash.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "selector",
    srcs = ["objc/selector.m"],
    tags = ["objc"],
)

objc_indexer_test(
    name = "single_file_category",
    srcs = ["objc/categories/single_file_category.m"],
    ignore_dups = True,
    tags = ["objc"],
)

objc_indexer_test(
    name = "subscript_ref",
    srcs = ["objc/subscript_ref.m"],
    ignore_dups = True,
    tags = ["objc"],
)

test_suite(
    name = "indexer_objc",
    tags = ["objc"],
)

cc_indexer_test(
    name = "df_binary",
    srcs = ["df/df_binary.cc"],
    ignore_dups = True,
    tags = ["df"],
)

cc_indexer_test(
    name = "df_field_ref_blame",
    srcs = ["df/df_field_ref_blame.cc"],
    ignore_dups = True,
    tags = ["df"],
)

cc_indexer_test(
    name = "df_field_rw",
    srcs = ["df/df_field_rw.cc"],
    tags = ["df"],
)

cc_indexer_test(
    name = "df_field_influence",
    srcs = ["df/df_field_influence.cc"],
    tags = ["df"],
)

cc_indexer_test(
    name = "df_fncall_influence",
    srcs = ["df/df_fncall_influence.cc"],
    ignore_dups = True,
    tags = ["df"],
)

cc_indexer_test(
    name = "df_fncall_influence_defaultargs",
    srcs = ["df/df_fncall_influence_defaultargs.cc"],
    ignore_dups = True,
    tags = ["df"],
)

cc_indexer_test(
    name = "df_fncall_influence_pack",
    size = "medium",
    srcs = ["df/df_fncall_influence_pack.cc"],
    ignore_dups = True,
    tags = ["df"],
)

cc_indexer_test(
    name = "df_fncall_influence_vararg",
    srcs = ["df/df_fncall_influence_vararg.cc"],
    ignore_dups = True,
    tags = ["df"],
)

cc_indexer_test(
    name = "df_operatoreq",
    srcs = ["df/df_operatoreq.cc"],
    ignore_dups = True,
    tags = ["df"],
)

cc_indexer_test(
    name = "df_operatorpluseq",
    srcs = ["df/df_operatorpluseq.cc"],
    ignore_dups = True,
    tags = ["df"],
)

cc_indexer_test(
    name = "df_operatorplusplus",
    srcs = ["df/df_operatorplusplus.cc"],
    ignore_dups = True,
    tags = ["df"],
)

cc_indexer_test(
    name = "df_ta_var",
    srcs = ["df/df_ta_var.cc"],
    ignore_dups = True,
    tags = ["df"],
)

cc_indexer_test(
    name = "df_unary",
    srcs = ["df/df_unary.cc"],
    ignore_dups = True,
    tags = ["df"],
)

cc_indexer_test(
    name = "df_var_ref_blame",
    srcs = ["df/df_var_ref_blame.cc"],
    tags = ["df"],
)

cc_indexer_test(
    name = "df_var_ref_file_blame",
    srcs = ["df/df_var_ref_file_blame.cc"],
    tags = ["df"],
)

cc_indexer_test(
    name = "df_var_rw",
    srcs = ["df/df_var_rw.cc"],
    tags = ["df"],
)

cc_indexer_test(
    name = "df_var_influence",
    srcs = ["df/df_var_influence.cc"],
    ignore_dups = True,
    tags = ["df"],
)

test_suite(
    name = "indexer_df",
    tags = ["df"],
)

cc_indexer_test(
    name = "toolchain_verify_no_diagnostics_test",
    srcs = ["toolchain/verify_no_diagnostics.cc"],
    copts = [
        "-Xclang",
        "-verify",
    ],
    tags = ["toolchain"],
)

test_suite(
    name = "indexer_toolchain",
    tags = ["toolchain"],
)
