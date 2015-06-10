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

. "${TEST_SRCDIR}/kythe/cxx/indexer/cxx/testdata/one_case.sh"

BASE_DIR="$TEST_SRCDIR/kythe/cxx/indexer/cxx/testdata/basic"

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
