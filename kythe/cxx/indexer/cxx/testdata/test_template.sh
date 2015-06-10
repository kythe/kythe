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

BASE_DIR="$TEST_SRCDIR/kythe/cxx/indexer/cxx/testdata/template"

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

exit ${HAD_ERRORS}
