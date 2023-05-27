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

#ifndef KYTHE_CXX_VERIFIER_ASSERTIONS_TO_SOUFFLE_
#define KYTHE_CXX_VERIFIER_ASSERTIONS_TO_SOUFFLE_

#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "kythe/cxx/verifier/assertion_ast.h"

namespace kythe::verifier {
/// \brief Packages together for a possibly multistage Souffle program.
class SouffleProgram {
 public:
  /// \brief Turns `goal_groups` into a Souffle program.
  /// \param symbol_table the symbol table used by `goal_groups`.
  /// \param goal_groups the goal groups to lower.
  bool Lower(const SymbolTable& symbol_table,
             const std::vector<GoalGroup>& goal_groups);

  /// \return the lowered Souffle code.
  absl::string_view code() { return code_; }

  /// \brief Configures whether to emit initial definitions.
  void set_emit_prelude(bool emit_prelude) { emit_prelude_ = emit_prelude; }

 private:
  /// \brief Lowers `node`.
  bool LowerSubexpression(AstNode* node);

  /// \brief Lowers `group`.
  bool LowerGoalGroup(const SymbolTable& symbol_table, const GoalGroup& group);

  /// \return a stable short name for `ev`.
  size_t FindEVar(EVar* ev) {
    auto found = evars_.find(ev);
    if (found != evars_.end()) return found->second;
    evars_[ev] = evars_.size();
    return evars_.size() - 1;
  }

  /// The current finished code buffer.
  std::string code_;

  /// Whether to emit the prelude.
  bool emit_prelude_ = true;

  /// Known evars.
  absl::flat_hash_map<EVar*, size_t> evars_;
};

}  // namespace kythe::verifier

#endif  // defined(KYTHE_CXX_VERIFIER_ASSERTIONS_TO_SOUFFLE_)
