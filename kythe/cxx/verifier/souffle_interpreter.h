/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef KYTHE_CXX_VERIFIER_SOUFFLE_INTERPRETER_H_
#define KYTHE_CXX_VERIFIER_SOUFFLE_INTERPRETER_H_

#include <optional>
#include <string_view>

#include "kythe/cxx/verifier/assertion_ast.h"

namespace kythe::verifier {
struct SouffleResult {
  bool success;
  size_t highest_goal_reached;
  size_t highest_group_reached;
};

/// \brief Runs the Souffle interpreter on parsed facts and goal groups.
/// \param symbol_table the symbol table for expanding idents
/// \param goal_groups the goal groups to solve
/// \param database the facts to solve against
/// \param anchors a map of anchors
/// \param inspections the list of EVars that have been marked explicitly
/// (`@foo ref Foo?`) or implicitly for inspection.
/// \param inspect the inspection callback that will be used against the
/// provided list of inspections; a false return value stops iterating through
/// inspection results and fails the solution, while a true result continues.
/// `value` is a string representation of the assignment of the EVar to
/// which the inspection refers.
/// \param get_symbol returns a string representation of the given symbol
/// \return a `SouffleResult` describing how the run went.
SouffleResult RunSouffle(
    const SymbolTable& symbol_table, const std::vector<GoalGroup>& goal_groups,
    const Database& database, const AnchorMap& anchors,
    const std::vector<Inspection>& inspections,
    std::function<bool(const Inspection&, std::string_view value)> inspect,
    std::function<std::string(size_t)> get_symbol);
}  // namespace kythe::verifier

#endif  // defined(KYTHE_CXX_VERIFIER_SOUFFLE_INTERPRETER_H_)
