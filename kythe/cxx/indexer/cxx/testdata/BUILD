package(default_visibility = ["//kythe:default_visibility"])

load("cxx_indexer_test", "cxx_indexer_test")

sh_test(
    name = "kindex",
    srcs = [
        "test_kindex.sh",
    ],
    data = [
        "kindex_repo_test.header",
        "kindex_repo_test.main",
        "kindex_repo_test.unit",
        "kindex_repo_test.verify",
        "kindex_test.header",
        "kindex_test.main",
        "kindex_test.unit",
        "kindex_test.verify",
        "//kythe/cxx/indexer/cxx:indexer",
        "//kythe/cxx/tools:kindex_tool",
        "//kythe/cxx/verifier",
    ],
)

sh_test(
    name = "index_pack",
    srcs = [
        "test_index_pack.sh",
    ],
    data = [
        "kindex_repo_test.header",
        "kindex_repo_test.main",
        "kindex_repo_test.unit",
        "kindex_repo_test.verify",
        "kindex_test.header",
        "kindex_test.main",
        "kindex_test.unit",
        "kindex_test.verify",
        "//kythe/cxx/indexer/cxx:indexer",
        "//kythe/cxx/tools:kindex_tool",
        "//kythe/cxx/verifier",
        "//kythe/go/platform/tools:indexpack",
    ],
)

sh_test(
    name = "bundle",
    srcs = [
        "test_bundle.sh",
    ],
    data = [
        "bundle_self_test.cc",
        "bundle_self_test_mix.cc",
        "bundle_self_test_multi_transcript.cc",
        "bundle_self_test_unclaimed.cc",
        "bundle_self_test_vnames_json.cc",
        "claim_macro_features.cc",
        "test_vnames.json",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/indexer/cxx:indexer",
        "//kythe/cxx/verifier",
    ],
)

cxx_indexer_test(
    name = "alias_alias_int",
    srcs = ["basic/alias_alias_int.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "alias_alias_ptr_int",
    srcs = ["basic/alias_alias_ptr_int.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "alias_and_cvr",
    srcs = ["basic/alias_and_cvr.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "alias_int",
    srcs = ["basic/alias_int.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "alias_int_twice",
    srcs = ["basic/alias_int_twice.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "anchor_utf8",
    srcs = ["basic/anchor_utf8.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "auto",
    srcs = ["basic/auto.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "auto_const_ref",
    srcs = ["basic/auto_const_ref.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "auto_multi",
    srcs = ["basic/auto_multi.cc"],
    ignore_dups = True,
    tags = ["basic"],
)

cxx_indexer_test(
    name = "auto_zoo",
    srcs = ["basic/auto_zoo.cc"],
    ignore_dups = True,
    tags = ["basic"],
)

cxx_indexer_test(
    name = "builtin_functions",
    srcs = ["basic/builtin_functions.c"],
    std = "gnu99",
    tags = ["basic"],
)

cxx_indexer_test(
    name = "builtin_functions_ctd",
    srcs = ["basic/builtin_functions_ctd.c"],
    std = "gnu99",
    tags = ["basic"],
)

cxx_indexer_test(
    name = "decltype_auto",
    srcs = ["basic/decltype_auto.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "decltype",
    srcs = ["basic/decltype.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "decltype_const_ref",
    srcs = ["basic/decltype_const_ref.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "decltype_parens",
    srcs = ["basic/decltype_parens.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "empty_case",
    srcs = ["basic/empty_case.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "enum_class_decl",
    srcs = ["basic/enum_class_decl.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "enum_class_element_decl",
    srcs = ["basic/enum_class_element_decl.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "enum_decl",
    srcs = ["basic/enum_decl.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "enum_decl_completes",
    srcs = ["basic/enum_decl_completes.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "enum_decl_ty",
    srcs = ["basic/enum_decl_ty.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "enum_decl_ty_completes",
    srcs = ["basic/enum_decl_ty_completes.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "enum_decl_ty_header_completes",
    srcs = ["basic/enum_decl_ty_header_completes.cc"],
    tags = ["basic"],
    deps = ["basic/enum_decl_ty_header_completes.h"],
)

cxx_indexer_test(
    name = "enum_element_decl",
    srcs = ["basic/enum_element_decl.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "file_content",
    srcs = ["basic/file_content.cc"],
    tags = ["basic"],
    deps = ["basic/a.h"],
)

cxx_indexer_test(
    name = "file_node",
    srcs = ["basic/file_node.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "file_node_reentrant",
    srcs = ["basic/file_node_reentrant.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "macros_builtin",
    srcs = ["basic/macros_builtin.c"],
    std = "c99",
    tags = ["basic"],
)

cxx_indexer_test(
    name = "macros_defn",
    srcs = ["basic/macros_defn.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "macros_expand",
    srcs = ["basic/macros_expand.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "macros_expand_transitive",
    srcs = ["basic/macros_expand_transitive.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "macros_if",
    srcs = ["basic/macros_if.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "macros_ifdef",
    srcs = ["basic/macros_ifdef.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "macros_if_defined",
    srcs = ["basic/macros_if_defined.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "macros_implicit",
    srcs = ["basic/macros_implicit.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "macros_include",
    srcs = ["basic/macros_include.cc"],
    tags = ["basic"],
    deps = ["basic/a.h"],
)

cxx_indexer_test(
    name = "macros_subst_one_level",
    srcs = ["basic/macros_subst_one_level.cc"],
    ignore_dups = True,
    tags = ["basic"],
)

cxx_indexer_test(
    name = "macros_undef",
    srcs = ["basic/macros_undef.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "typedef_class_anon_ns",
    srcs = ["basic/typedef_class_anon_ns.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "typedef_class",
    srcs = ["basic/typedef_class.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "typedef_class_nested_ns",
    srcs = ["basic/typedef_class_nested_ns.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "typedef_const_int",
    srcs = ["basic/typedef_const_int.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "typedef_int",
    srcs = ["basic/typedef_int.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "typedef_nested_class",
    srcs = ["basic/typedef_nested_class.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "typedef_paren",
    srcs = ["basic/typedef_paren.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "typedef_ptr_int_canonicalized",
    srcs = ["basic/typedef_ptr_int_canonicalized.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "typedef_ptr_int",
    srcs = ["basic/typedef_ptr_int.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "typedef_same",
    srcs = ["basic/typedef_same.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "typeof_param",
    srcs = ["basic/typeof_param.c"],
    ignore_unimplemented = True,
    std = "gnu99",
    tags = ["basic"],
)

cxx_indexer_test(
    name = "vardecl_double_shadowed_local_anchor",
    srcs = ["basic/vardecl_double_shadowed_local_anchor.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "vardecl_global_anchor",
    srcs = ["basic/vardecl_global_anchor.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "vardecl_global_anon_ns_anchor",
    srcs = ["basic/vardecl_global_anon_ns_anchor.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "vardecl_global_tu_anchor",
    srcs = ["basic/vardecl_global_tu_anchor.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "vardecl_local_anchor",
    srcs = ["basic/vardecl_local_anchor.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "vardecl_shadowed_local_anchor",
    srcs = ["basic/vardecl_shadowed_local_anchor.cc"],
    tags = ["basic"],
)

cxx_indexer_test(
    name = "wild_std_allocator",
    srcs = ["basic/wild_std_allocator.cc"],
    tags = ["basic"],
)

test_suite(
    name = "indexer_basic",
    tags = ["basic"],
)

cxx_indexer_test(
    name = "function_args_decl",
    srcs = ["function/function_args_decl.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_args_defn",
    srcs = ["function/function_args_defn.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_auto_return",
    srcs = ["function/function_auto_return.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_decl",
    srcs = ["function/function_decl.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_decl_completes",
    srcs = ["function/function_decl_completes.cc"],
    tags = ["function"],
    deps = ["function/void_f.h"],
)

cxx_indexer_test(
    name = "function_defn_call",
    srcs = ["function/function_defn_call.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_defn",
    srcs = ["function/function_defn.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_direct_call",
    srcs = ["function/function_direct_call.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_knr_ty",
    srcs = ["function/function_knr_ty.c"],
    std = "c99",
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_lambda",
    srcs = ["function/function_lambda.cc"],
    ignore_dups = True,
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_operator_overload_dup_names",
    srcs = ["function/function_operator_overload_dup_names.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_operator_overload_enable_if",
    srcs = ["function/function_operator_overload_enable_if.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_operator_overload_names",
    srcs = ["function/function_operator_overload_names.cc"],
    ignore_dups = True,
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_operator_parens_call",
    srcs = ["function/function_operator_parens_call.cc"],
    ignore_dups = True,
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_operator_parens",
    srcs = ["function/function_operator_parens.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_operator_parens_overload_call",
    srcs = ["function/function_operator_parens_overload_call.cc"],
    ignore_dups = True,
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_operator_parens_overload",
    srcs = ["function/function_operator_parens_overload.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_overload_call",
    srcs = ["function/function_overload_call.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_overload",
    srcs = ["function/function_overload.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_ptr_ty",
    srcs = ["function/function_ptr_ty.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_ty",
    srcs = ["function/function_ty.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_vararg",
    srcs = ["function/function_vararg.cc"],
    tags = ["function"],
)

cxx_indexer_test(
    name = "function_vararg_ty",
    srcs = ["function/function_vararg_ty.cc"],
    tags = ["function"],
)

test_suite(
    name = "indexer_function",
    tags = ["function"],
)

cxx_indexer_test(
    name = "flags_bool",
    srcs = ["libraries/flags_bool.cc"],
    ignore_dups = True,
    tags = ["libraries"],
    deps = ["libraries/flags_bool.h"] + glob(["libraries/gflags*.h"]),
)

cxx_indexer_test(
    name = "flags_decls",
    srcs = ["libraries/flags_decls.cc"],
    tags = ["libraries"],
    deps = glob(["libraries/gflags*.h"]),
)

cxx_indexer_test(
    name = "flags_defns",
    srcs = ["libraries/flags_defns.cc"],
    ignore_dups = True,
    tags = ["libraries"],
    deps = glob(["libraries/gflags*.h"]),
)

cxx_indexer_test(
    name = "flags_empty",
    srcs = ["libraries/flags_empty.cc"],
    tags = ["libraries"],
    deps = glob(["libraries/gflags*.h"]),
)

cxx_indexer_test(
    name = "flags_ref_int64_decl",
    srcs = ["libraries/flags_ref_int64_decl.cc"],
    tags = ["libraries"],
    deps = glob(["libraries/gflags*.h"]),
)

cxx_indexer_test(
    name = "flags_ref_int64_defn",
    srcs = ["libraries/flags_ref_int64_defn.cc"],
    ignore_dups = True,
    tags = ["libraries"],
    deps = glob(["libraries/gflags*.h"]),
)

cxx_indexer_test(
    name = "flags_ref_int64_defn_completes",
    srcs = ["libraries/flags_ref_int64_defn_completes.cc"],
    ignore_dups = True,
    tags = ["libraries"],
    deps = glob(["libraries/gflags*.h"]),
)

cxx_indexer_test(
    name = "flags_string",
    srcs = ["libraries/flags_string.cc"],
    ignore_dups = True,
    tags = ["libraries"],
    deps = ["libraries/flags_string.h"] + glob(["libraries/gflags*.h"]),
)

test_suite(
    name = "indexer_libraries",
    tags = ["libraries"],
)

cxx_indexer_test(
    name = "rec_anon_struct",
    srcs = ["rec/rec_anon_struct.cc"],
    ignore_dups = True,
    tags = ["rec"],
)

cxx_indexer_test(
    name = "rec_class",
    srcs = ["rec/rec_class.cc"],
    tags = ["rec"],
)

cxx_indexer_test(
    name = "rec_class_base",
    srcs = ["rec/rec_class_base.cc"],
    ignore_dups = True,
    tags = ["rec"],
)

cxx_indexer_test(
    name = "rec_class_base_dependent",
    srcs = ["rec/rec_class_base_dependent.cc"],
    tags = ["rec"],
)

cxx_indexer_test(
    name = "rec_class_base_recurring",
    srcs = ["rec/rec_class_base_recurring.cc"],
    tags = ["rec"],
)

cxx_indexer_test(
    name = "rec_class_header_completes",
    srcs = ["rec/rec_class_header_completes.cc"],
    tags = ["rec"],
    deps = ["rec/rec_class_header_completes.h"],
)

cxx_indexer_test(
    name = "rec_class_macro",
    srcs = ["rec/rec_class_macro.cc"],
    tags = ["rec"],
)

cxx_indexer_test(
    name = "rec_fwd_struct_name",
    srcs = ["rec/rec_fwd_struct_name.c"],
    std = "c99",
    tags = ["rec"],
)

cxx_indexer_test(
    name = "rec_inner_class",
    srcs = ["rec/rec_inner_class.cc"],
    tags = ["rec"],
)

cxx_indexer_test(
    name = "rec_struct_c99",
    srcs = ["rec/rec_struct.c"],
    std = "c99",
    tags = ["rec"],
)

cxx_indexer_test(
    name = "rec_struct",
    srcs = ["rec/rec_struct.cc"],
    tags = ["rec"],
)

cxx_indexer_test(
    name = "rec_struct_decl",
    srcs = ["rec/rec_struct_decl.cc"],
    tags = ["rec"],
)

cxx_indexer_test(
    name = "rec_union",
    srcs = ["rec/rec_union.cc"],
    tags = ["rec"],
)

test_suite(
    name = "indexer_rec",
    tags = ["rec"],
)

cxx_indexer_test(
    name = "template_alias_implicit_instantiation",
    srcs = ["template/template_alias_implicit_instantiation.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_alias_implicit_instantiation_decls",
    srcs = ["template/template_alias_implicit_instantiation_decls.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_arg_multiple_typename",
    srcs = ["template/template_arg_multiple_typename.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_arg_typename",
    srcs = ["template/template_arg_typename.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_class_defn",
    srcs = ["template/template_class_defn.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_class_fn_spec",
    srcs = ["template/template_class_fn_spec.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_class_inner",
    srcs = ["template/template_class_inner.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_class_inst_explicit",
    srcs = ["template/template_class_inst_explicit.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_class_inst_implicit",
    srcs = ["template/template_class_inst_implicit.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_class_inst_implicit_dependent",
    srcs = ["template/template_class_inst_implicit_dependent.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_class_ref_ps",
    srcs = ["template/template_class_ref_ps.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_class_skip_implicit_on",
    srcs = ["template/template_class_skip_implicit_on.cc"],
    index_template_instantiations = False,
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_depname_class",
    srcs = ["template/template_depname_class.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_depname_inst_class",
    srcs = ["template/template_depname_inst_class.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_depname_path_graph",
    srcs = ["template/template_depname_path_graph.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_fn_decl",
    srcs = ["template/template_fn_decl.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_fn_decl_defn",
    srcs = ["template/template_fn_decl_defn.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_fn_defn",
    srcs = ["template/template_fn_defn.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_fn_explicit_spec_completes",
    srcs = ["template/template_fn_explicit_spec_completes.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_fn_explicit_spec_with_default_completes",
    srcs = ["template/template_fn_explicit_spec_with_default_completes.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_fn_implicit_spec",
    srcs = ["template/template_fn_implicit_spec.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_fn_multi_decl_def",
    srcs = ["template/template_fn_multi_decl_def.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_fn_multiple_implicit_spec",
    srcs = ["template/template_fn_multiple_implicit_spec.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_fn_overload",
    srcs = ["template/template_fn_overload.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_fn_spec",
    srcs = ["template/template_fn_spec.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_fn_spec_decl",
    srcs = ["template/template_fn_spec_decl.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_fn_spec_defn_decl",
    srcs = ["template/template_fn_spec_defn_decl.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_fn_spec_defn_defn_decl_decl",
    srcs = ["template/template_fn_spec_defn_defn_decl_decl.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_fn_spec_overload",
    srcs = ["template/template_fn_spec_overload.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_instance_type_from_class",
    srcs = ["template/template_instance_type_from_class.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_multilevel_argument",
    srcs = ["template/template_multilevel_argument.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_ps_completes",
    srcs = ["template/template_ps_completes.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_ps_decl",
    srcs = ["template/template_ps_decl.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_ps_defn",
    srcs = ["template/template_ps_defn.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_ps_multiple_decl",
    srcs = ["template/template_ps_multiple_decl.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_ps_twovar_decl",
    srcs = ["template/template_ps_twovar_decl.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_redundant_constituents_anchored",
    srcs = ["template/template_redundant_constituents_anchored.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_two_arg_spec",
    srcs = ["template/template_two_arg_spec.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_ty_typename",
    srcs = ["template/template_ty_typename.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_var_decl",
    srcs = ["template/template_var_decl.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_var_defn",
    srcs = ["template/template_var_defn.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_var_defn_completes",
    srcs = ["template/template_var_defn_completes.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_var_explicit_spec",
    srcs = ["template/template_var_explicit_spec.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_var_implicit_spec",
    srcs = ["template/template_var_implicit_spec.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_var_ps",
    srcs = ["template/template_var_ps.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_var_ps_completes",
    srcs = ["template/template_var_ps_completes.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_var_ps_implicit_spec",
    srcs = ["template/template_var_ps_implicit_spec.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_var_ref_ps",
    srcs = ["template/template_var_ref_ps.cc"],
    tags = ["template"],
)

cxx_indexer_test(
    name = "template_var_constexpr",
    srcs = ["template/template_var_constexpr.cc"],
    std = "c++14",
    tags = ["template"],
)

test_suite(
    name = "indexer_template",
    tags = ["template"],
)

# Required by cxx_indexer_test macro.
sh_library(
    name = "one_case",
    srcs = ["one_case.sh"],
    data = [
        "//kythe/cxx/indexer/cxx:indexer",
        "//kythe/cxx/verifier",
    ],
)

sh_binary(
    name = "run_case",
    srcs = ["run_case.sh"],
    deps = [":one_case"],
)
