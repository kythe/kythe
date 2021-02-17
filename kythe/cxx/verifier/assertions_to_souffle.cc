/*
 * Copyright 2021 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/verifier/assertions_to_souffle.h"

namespace kythe::verifier {

/// \brief Turns `goal_groups` into a Souffle program.
/// \param symbol_table the symbol table used by `goal_groups`.
/// \param goal_groups the goal groups to lower.
std::string LowerGoalsToSouffle(const SymbolTable& symbol_table,
                                const std::vector<GoalGroup>& goal_groups) {
  return "(unimplemented)";
}

}  // namespace kythe::verifier
