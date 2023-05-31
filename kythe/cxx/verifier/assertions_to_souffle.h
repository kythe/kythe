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

#include <optional>
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
  /// The type of an EVar.
  enum class EVarType { kVName, kSymbol, kUnknown };

  struct EVarRecord {
    size_t index;        ///< The Souffle index for the evar.
    bool only_negative;  ///< Whether this evar only appears negatively.
    EVarType type;
  };

  /// \brief Lowers `node`.
  /// \param type expected subexpression type
  /// \param pos whether this is a positive context
  bool LowerSubexpression(bool pos, EVarType type, AstNode* node);

  /// \brief Lowers `group`.
  bool LowerGoalGroup(const SymbolTable& symbol_table, const GoalGroup& group);

  /// The current finished code buffer.
  std::string code_;

  /// Whether to emit the prelude.
  bool emit_prelude_ = true;

  /// \param positive_context whether `evar` is being used positively.
  /// \param type the type used for `evar`.
  /// \return a stable short name for `evar`.
  size_t FindEVar(EVar* evar, bool positive_context, EVarType type) {
    auto ev = evars_.try_emplace(
        evar, EVarRecord{evars_.size(), !positive_context, type});
    if (!ev.second) {
      ev.first->second.only_negative &= !positive_context;
      if (ev.first->second.type != EVarType::kUnknown) {
        CHECK(ev.first->second.type == type);
      } else if (type != EVarType::kUnknown) {
        ev.first->second.type = type;
      }
    }
    return ev.first->second.index;
  }

  void UnifyEVarTypes(EVar* v1, EVar* v2) {
    auto ev1 = evars_.find(v1);
    auto ev2 = evars_.find(v2);
    CHECK(ev1 != evars_.end() && ev2 != evars_.end());
    if (ev1->second.type == EVarType::kUnknown) {
      ev1->second.type = ev2->second.type;
    } else if (ev2->second.type == EVarType::kUnknown) {
      ev2->second.type = ev1->second.type;
    } else {
      CHECK(ev1->second.type == ev2->second.type);
    }
  }

  /// Known evars.
  absl::flat_hash_map<EVar*, EVarRecord> evars_;
};

}  // namespace kythe::verifier

#endif  // defined(KYTHE_CXX_VERIFIER_ASSERTIONS_TO_SOUFFLE_)
