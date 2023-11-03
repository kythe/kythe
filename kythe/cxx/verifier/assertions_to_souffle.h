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

#include <cstddef>
#include <optional>
#include <string>

#include "absl/base/attributes.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/string_view.h"
#include "kythe/cxx/verifier/assertion_ast.h"

namespace kythe::verifier {
/// \brief Inferred type for an EVar.
enum class EVarType { kVName, kSymbol, kUnknown };

/// The error recovery state for a particular list of goal groups.
class SouffleErrorState {
 public:
  /// Creates a new error state for `goal_groups`. `goal_groups` should equal or
  /// exceed the lifetime of any bound `SouffleErrorState`.
  explicit SouffleErrorState(
      const std::vector<GoalGroup>* goal_groups ABSL_ATTRIBUTE_LIFETIME_BOUND)
      : goal_groups_(goal_groups) {}
  SouffleErrorState(const SouffleErrorState&) = delete;
  SouffleErrorState& operator=(const SouffleErrorState&) = delete;
  /// Advances the error state by one step.
  /// \return true if the solver should continue to run.
  bool NextStep();
  /// \return the current highest group to solve (or -1 if uninitialized).
  int target_group() const { return target_group_; }
  /// \return the current highest goal to solve (ignored if target_group is
  /// invalid).
  int target_goal() const { return target_goal_; }
  /// \return whether `group` (and subsequent groups) have already been
  /// explored during error recovery.
  bool IsFinished(int group) const {
    return target_group_ >= 0 && group > target_group_;
  }
  /// \return the target goal for `group`, or -1 if all goals should be
  /// explored given a valid `group`.
  int GoalForGroup(int group) const {
    return group == target_group_ ? target_goal_ : -1;
  }

 private:
  /// The goal groups being solved. Not owned.
  const std::vector<GoalGroup>* goal_groups_;
  /// The current highest group to solve (or -1 if uninitialized).
  int target_group_ = -1;
  /// The current highest goal to solve (ignored if target_group_ is invalid).
  int target_goal_ = -1;
};

/// \brief Packages together for a possibly multistage Souffle program.
class SouffleProgram {
 public:
  /// \brief Turns `goal_groups` into a Souffle program.
  /// \param symbol_table the symbol table used by `goal_groups`.
  /// \param goal_groups the goal groups to lower.
  /// \param inspections inspections to perform.
  bool Lower(const SymbolTable& symbol_table,
             const std::vector<GoalGroup>& goal_groups,
             const std::vector<Inspection>& inspections,
             const SouffleErrorState& error_state);

  /// \return the lowered Souffle code.
  absl::string_view code() { return code_; }

  /// \brief Configures whether to emit initial definitions.
  void set_emit_prelude(bool emit_prelude) { emit_prelude_ = emit_prelude; }

  std::optional<EVarType> EVarTypeFor(EVar* e) const {
    const auto i = evar_types_.find(e);
    return i == evar_types_.end() ? std::nullopt
                                  : std::make_optional(i->second);
  }

  const std::vector<EVar*>& inspections() const { return inspections_; }

 private:
  /// \brief Lowers `node`.
  /// \param positive_cxt whether we are in a positive goal context.
  bool LowerSubexpression(AstNode* node, EVarType type, bool positive_cxt);

  /// \brief Lowers a goal from `goal`.
  /// \param positive_cxt whether we are in a positive goal context.
  bool LowerGoal(const SymbolTable& symbol_table, AstNode* goal,
                 bool positive_cxt);

  /// \brief Lowers `group`.
  bool LowerGoalGroup(const SymbolTable& symbol_table, const GoalGroup& group,
                      int target_goal);

  /// \brief Assigns or checks `evar`'s type against `type`.
  bool AssignEVarType(EVar* evar, EVarType type);

  /// \brief Assigns or checks `evar`'s type against `oevar`'s type.
  bool AssignEVarType(EVar* evar, EVar* oevar);

  /// The current finished code buffer.
  std::string code_;

  /// Whether to emit the prelude.
  bool emit_prelude_ = true;

  /// \return a stable short name for `evar`.
  size_t FindEVar(EVar* evar) {
    return evars_.try_emplace(evar, evars_.size()).first->second;
  }

  /// An association between an EVar and a Datalog variable.
  struct FoundEVar {
    /// The Datalog id of the EVar.
    size_t id;
    /// Whether this was the first time this EVar was seen.
    bool is_fresh;
  };

  /// \return a stable short name for `evar` and whether this was the first time
  /// it was seen.
  FoundEVar FindFreshEVar(EVar* evar) {
    auto id = FindEVar(evar);
    return {.id = id, .is_fresh = id == evars_.size() - 1};
  }

  /// Known evars.
  absl::flat_hash_map<EVar*, size_t> evars_;

  /// Evars that first appear in a negated context.
  absl::flat_hash_set<EVar*> negated_evars_;

  /// EVars appearing in output positions (as inspections).
  std::vector<EVar*> inspections_;

  /// Known EVar typings.
  absl::flat_hash_map<EVar*, EVarType> evar_types_;
};

}  // namespace kythe::verifier

#endif  // defined(KYTHE_CXX_VERIFIER_ASSERTIONS_TO_SOUFFLE_)
