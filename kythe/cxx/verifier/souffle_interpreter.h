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

#include "kythe/cxx/verifier/assertions.h"

namespace kythe::verifier {
class Verifier;
/// \brief Runs the Souffle interpreter on parsed facts and goal groups.
/// \param symbol_table the symbol table for expanding idents
/// \param goal_groups the goal groups to solve
/// \param database the facts to solve against
/// \param verifier the verifier to pass to the inspection
/// \param inspect the inspection to use
/// \param highest_goal_reached will be set to the highest goal reached
/// \param highest_group_reached will be set to the highest group reached
/// \return true if all goals could be solved
bool RunSouffle(
    const SymbolTable& symbol_table, const std::vector<GoalGroup>& goal_groups,
    const Database& database, Verifier* verifier,
    std::function<bool(Verifier*, const AssertionParser::Inspection&)> inspect,
    size_t& highest_goal_reached, size_t& highest_group_reached);
}  // namespace kythe::verifier

#endif  // defined(KYTHE_CXX_VERIFIER_SOUFFLE_INTERPRETER_H_)
