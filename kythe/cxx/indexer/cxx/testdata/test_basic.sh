#!/bin/bash

# Copyright 2015 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This script runs the indexer on various test cases, piping the results
# to the verifier. The test cases contain assertions for the verifier to
# verify. Should every case succeed, this script returns zero.
HAD_ERRORS=0
BASE_DIR="$TEST_SRCDIR/kythe/cxx/indexer/cxx/testdata/basic"
VERIFIER="kythe/cxx/verifier/verifier"
INDEXER="kythe/cxx/indexer/cxx/indexer"
# one_case test-file clang-standard verifier-argument indexer-argument
function one_case {
  ${INDEXER} -i $1 $4 -- -std=$2 | ${VERIFIER} $1 $3
  RESULTS=( ${PIPESTATUS[0]} ${PIPESTATUS[1]} )
  if [ ${RESULTS[0]} -ne 0 ]; then
    echo "[ FAILED INDEX: $1 ]"
    HAD_ERRORS=1
  elif [ ${RESULTS[1]} -ne 0 ]; then
    echo "[ FAILED VERIFY: $1 ]"
    HAD_ERRORS=1
  else
    echo "[ OK: $1 ]"
  fi
}

one_case "${BASE_DIR}/alias_alias_int.cc" "c++1y"
one_case "${BASE_DIR}/alias_alias_ptr_int.cc" "c++1y"
one_case "${BASE_DIR}/alias_and_cvr.cc" "c++1y"
one_case "${BASE_DIR}/alias_int.cc" "c++1y"
one_case "${BASE_DIR}/alias_int_twice.cc" "c++1y"
one_case "${BASE_DIR}/anchor_utf8.cc" "c++1y"
one_case "${BASE_DIR}/auto.cc" "c++1y"
one_case "${BASE_DIR}/auto_const_ref.cc" "c++1y"
one_case "${BASE_DIR}/auto_multi.cc" "c++1y" --ignore_dups=true
one_case "${BASE_DIR}/auto_zoo.cc" "c++1y" --ignore_dups=true
one_case "${BASE_DIR}/builtin_functions.c" "gnu99"
one_case "${BASE_DIR}/builtin_functions_ctd.c" "gnu99"
one_case "${BASE_DIR}/decltype_auto.cc" "c++1y"
one_case "${BASE_DIR}/decltype.cc" "c++1y"
one_case "${BASE_DIR}/decltype_const_ref.cc" "c++1y"
one_case "${BASE_DIR}/decltype_parens.cc" "c++1y"
one_case "${BASE_DIR}/empty_case.cc" "c++1y"
one_case "${BASE_DIR}/enum_class_decl.cc" "c++1y"
one_case "${BASE_DIR}/enum_class_element_decl.cc" "c++1y"
one_case "${BASE_DIR}/enum_decl.cc" "c++1y"
one_case "${BASE_DIR}/enum_decl_completes.cc" "c++1y"
one_case "${BASE_DIR}/enum_decl_ty.cc" "c++1y"
one_case "${BASE_DIR}/enum_decl_ty_completes.cc" "c++1y"
one_case "${BASE_DIR}/enum_decl_ty_header_completes.cc" "c++1y"
one_case "${BASE_DIR}/enum_element_decl.cc" "c++1y"
one_case "${BASE_DIR}/file_content.cc" "c++1y"
one_case "${BASE_DIR}/file_node.cc" "c++1y"
one_case "${BASE_DIR}/file_node_reentrant.cc" "c++1y"
one_case "${BASE_DIR}/function_args_decl.cc" "c++1y"
one_case "${BASE_DIR}/function_args_defn.cc" "c++1y"
one_case "${BASE_DIR}/function_auto_return.cc" "c++1y"
one_case "${BASE_DIR}/function_decl.cc" "c++1y"
one_case "${BASE_DIR}/function_decl_completes.cc" "c++1y"
one_case "${BASE_DIR}/function_defn_call.cc" "c++1y"
one_case "${BASE_DIR}/function_defn.cc" "c++1y"
one_case "${BASE_DIR}/function_direct_call.cc" "c++1y"
one_case "${BASE_DIR}/function_knr_ty.c" "c99"
one_case "${BASE_DIR}/function_lambda.cc" "c++1y" --ignore_dups=true
one_case "${BASE_DIR}/function_operator_overload_dup_names.cc" "c++1y"
one_case "${BASE_DIR}/function_operator_overload_enable_if.cc" "c++1y"
one_case "${BASE_DIR}/function_operator_overload_names.cc" "c++1y" --ignore_dups=true
one_case "${BASE_DIR}/function_operator_parens_call.cc" "c++1y" --ignore_dups=true
one_case "${BASE_DIR}/function_operator_parens.cc" "c++1y"
one_case "${BASE_DIR}/function_operator_parens_overload_call.cc" "c++1y" --ignore_dups=true
one_case "${BASE_DIR}/function_operator_parens_overload.cc" "c++1y"
one_case "${BASE_DIR}/function_overload_call.cc" "c++1y"
one_case "${BASE_DIR}/function_overload.cc" "c++1y"
one_case "${BASE_DIR}/function_ptr_ty.cc" "c++1y"
one_case "${BASE_DIR}/function_ty.cc" "c++1y"
one_case "${BASE_DIR}/function_vararg.cc" "c++1y"
one_case "${BASE_DIR}/function_vararg_ty.cc" "c++1y"
one_case "${BASE_DIR}/macros_builtin.c" "c99"
one_case "${BASE_DIR}/macros_defn.cc" "c++1y"
one_case "${BASE_DIR}/macros_expand.cc" "c++1y"
one_case "${BASE_DIR}/macros_expand_transitive.cc" "c++1y"
one_case "${BASE_DIR}/macros_if.cc" "c++1y"
one_case "${BASE_DIR}/macros_ifdef.cc" "c++1y"
one_case "${BASE_DIR}/macros_if_defined.cc" "c++1y"
one_case "${BASE_DIR}/macros_implicit.cc" "c++1y"
one_case "${BASE_DIR}/macros_include.cc" "c++1y"
one_case "${BASE_DIR}/macros_subst_one_level.cc" "c++1y" --ignore_dups=true
one_case "${BASE_DIR}/macros_undef.cc" "c++1y"
one_case "${BASE_DIR}/rec_anon_struct.cc" "c++1y" --ignore_dups=true
one_case "${BASE_DIR}/rec_class.cc" "c++1y"
one_case "${BASE_DIR}/rec_class_base.cc" "c++1y" --ignore_dups=true
one_case "${BASE_DIR}/rec_class_base_dependent.cc" "c++1y"
one_case "${BASE_DIR}/rec_class_base_recurring.cc" "c++1y"
one_case "${BASE_DIR}/rec_class_header_completes.cc" "c++1y"
one_case "${BASE_DIR}/rec_class_macro.cc" "c++1y"
one_case "${BASE_DIR}/rec_struct.c" "c99"
one_case "${BASE_DIR}/rec_struct.cc" "c++1y"
one_case "${BASE_DIR}/rec_struct_decl.cc" "c++1y"
one_case "${BASE_DIR}/rec_union.cc" "c++1y"
one_case "${BASE_DIR}/template_alias_implicit_instantiation.cc" "c++1y"
one_case "${BASE_DIR}/template_alias_implicit_instantiation_decls.cc" "c++1y"
one_case "${BASE_DIR}/template_arg_multiple_typename.cc" "c++1y"
one_case "${BASE_DIR}/template_arg_typename.cc" "c++1y"
one_case "${BASE_DIR}/template_class_defn.cc" "c++1y"
one_case "${BASE_DIR}/template_class_fn_spec.cc" "c++1y"
one_case "${BASE_DIR}/template_class_inst_explicit.cc" "c++1y"
one_case "${BASE_DIR}/template_class_inst_implicit.cc" "c++1y"
one_case "${BASE_DIR}/template_class_inst_implicit_dependent.cc" "c++1y"
one_case "${BASE_DIR}/template_class_ref_ps.cc" "c++1y"
one_case "${BASE_DIR}/template_class_skip_implicit_on.cc" "c++1y" "" "--index_template_instantiations=false"
one_case "${BASE_DIR}/template_depname_class.cc" "c++1y"
one_case "${BASE_DIR}/template_depname_inst_class.cc" "c++1y"
one_case "${BASE_DIR}/template_depname_path_graph.cc" "c++1y"
one_case "${BASE_DIR}/template_fn_decl.cc" "c++1y"
one_case "${BASE_DIR}/template_fn_decl_defn.cc" "c++1y"
one_case "${BASE_DIR}/template_fn_defn.cc" "c++1y"
one_case "${BASE_DIR}/template_fn_explicit_spec_completes.cc" "c++1y"
one_case "${BASE_DIR}/template_fn_explicit_spec_with_default_completes.cc" "c++1y"
one_case "${BASE_DIR}/template_fn_implicit_spec.cc" "c++1y"
one_case "${BASE_DIR}/template_fn_multi_decl_def.cc" "c++1y"
one_case "${BASE_DIR}/template_fn_multiple_implicit_spec.cc" "c++1y"
one_case "${BASE_DIR}/template_fn_overload.cc" "c++1y"
one_case "${BASE_DIR}/template_fn_spec.cc" "c++1y"
one_case "${BASE_DIR}/template_fn_spec_decl.cc" "c++1y"
one_case "${BASE_DIR}/template_fn_spec_defn_decl.cc" "c++1y"
one_case "${BASE_DIR}/template_fn_spec_defn_defn_decl_decl.cc" "c++1y"
one_case "${BASE_DIR}/template_fn_spec_overload.cc" "c++1y"
one_case "${BASE_DIR}/template_instance_type_from_class.cc" "c++1y"
one_case "${BASE_DIR}/template_multilevel_argument.cc" "c++1y"
one_case "${BASE_DIR}/template_ps_completes.cc" "c++1y"
one_case "${BASE_DIR}/template_ps_decl.cc" "c++1y"
one_case "${BASE_DIR}/template_ps_defn.cc" "c++1y"
one_case "${BASE_DIR}/template_ps_multiple_decl.cc" "c++1y"
one_case "${BASE_DIR}/template_ps_twovar_decl.cc" "c++1y"
one_case "${BASE_DIR}/template_redundant_constituents_anchored.cc" "c++1y"
one_case "${BASE_DIR}/template_two_arg_spec.cc" "c++1y"
one_case "${BASE_DIR}/template_ty_typename.cc" "c++1y"
one_case "${BASE_DIR}/template_var_decl.cc" "c++1y"
one_case "${BASE_DIR}/template_var_defn.cc" "c++1y"
one_case "${BASE_DIR}/template_var_defn_completes.cc" "c++1y"
one_case "${BASE_DIR}/template_var_explicit_spec.cc" "c++1y"
one_case "${BASE_DIR}/template_var_implicit_spec.cc" "c++1y"
one_case "${BASE_DIR}/template_var_ps.cc" "c++1y"
one_case "${BASE_DIR}/template_var_ps_completes.cc" "c++1y"
one_case "${BASE_DIR}/template_var_ps_implicit_spec.cc" "c++1y"
one_case "${BASE_DIR}/template_var_ref_ps.cc" "c++1y"
one_case "${BASE_DIR}/template_var_constexpr.cc" "c++14"
one_case "${BASE_DIR}/typedef_class_anon_ns.cc" "c++1y"
one_case "${BASE_DIR}/typedef_class.cc" "c++1y"
one_case "${BASE_DIR}/typedef_class_nested_ns.cc" "c++1y"
one_case "${BASE_DIR}/typedef_const_int.cc" "c++1y"
one_case "${BASE_DIR}/typedef_int.cc" "c++1y"
one_case "${BASE_DIR}/typedef_nested_class.cc" "c++1y"
one_case "${BASE_DIR}/typedef_paren.cc" "c++1y"
one_case "${BASE_DIR}/typedef_ptr_int_canonicalized.cc" "c++1y"
one_case "${BASE_DIR}/typedef_ptr_int.cc" "c++1y"
one_case "${BASE_DIR}/typedef_same.cc" "c++1y"
one_case "${BASE_DIR}/typeof_param.c" "gnu99" "" "--ignore_unimplemented"
one_case "${BASE_DIR}/vardecl_double_shadowed_local_anchor.cc" "c++1y"
one_case "${BASE_DIR}/vardecl_global_anchor.cc" "c++1y"
one_case "${BASE_DIR}/vardecl_global_anon_ns_anchor.cc" "c++1y"
one_case "${BASE_DIR}/vardecl_global_tu_anchor.cc" "c++1y"
one_case "${BASE_DIR}/vardecl_local_anchor.cc" "c++1y"
one_case "${BASE_DIR}/vardecl_shadowed_local_anchor.cc" "c++1y"
one_case "${BASE_DIR}/wild_std_allocator.cc" "c++1y"

exit ${HAD_ERRORS}
