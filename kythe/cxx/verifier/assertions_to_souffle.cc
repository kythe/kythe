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

#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/substitute.h"
#include "kythe/cxx/verifier/assertion_ast.h"
#include "kythe/cxx/verifier/pretty_printer.h"

// Define DEBUG_LOWERING for debug output. This uses the pretty printer.

namespace kythe::verifier {
namespace {
constexpr absl::string_view kGlobalDecls = R"(
.type vname = [
  signature:number,
  corpus:number,
  root:number,
  path:number,
  language:number
]
.decl sym(id:number)
sym(0).
sym(n + 1) :- sym(n), n >= 1.
.decl entry(source:vname, kind:number, target:vname, name:number, value:number)
.input entry(IO=kythe)
.decl anchor(begin:number, end:number, vname:vname)
.input anchor(IO=kythe, anchors=1)
.decl at(startsym:number, endsym:number, vname:vname)
at(s, e, v) :- entry(v, $0, nil, $1, $2),
               entry(v, $0, nil, $3, s),
               entry(v, $0, nil, $4, e).
$5
.decl result($6)
result($7) :- true$8
)";
}  // namespace

bool SouffleErrorState::NextStep() {
  if (goal_groups_->empty()) return false;
  if (target_group_ < 0) {
    target_group_ = static_cast<int>(goal_groups_->size()) - 1;
    target_goal_ = (*goal_groups_)[target_group_].goals.size();
  }
  --target_goal_;
  if (target_goal_ >= 0) {
    return true;
  }
  // We need to handle empty groups.
  while (target_goal_ < 0) {
    --target_group_;
    if (target_group_ < 0) {
      return false;
    }
    target_goal_ =
        static_cast<int>((*goal_groups_)[target_group_].goals.size()) - 1;
  }
  // target_group_ and target_goal_ >= 0.
  return true;
}

bool SouffleProgram::LowerSubexpression(AstNode* node, EVarType type,
                                        bool positive_cxt) {
  if (auto* app = node->AsApp()) {
    auto* tup = app->rhs()->AsTuple();
    absl::StrAppend(&code_, "[");
    for (size_t p = 0; p < 5; ++p) {
      if (p != 0) {
        absl::StrAppend(&code_, ", ");
      }
      if (!LowerSubexpression(tup->element(p), EVarType::kSymbol,
                              positive_cxt)) {
        return false;
      }
    }
    absl::StrAppend(&code_, "]");
    return true;
  } else if (auto* id = node->AsIdentifier()) {
    absl::StrAppend(&code_, id->symbol());
    return true;
  } else if (auto* evar = node->AsEVar()) {
    if (!AssignEVarType(evar, type)) return false;
    if (auto* evc = evar->current()) {
      return LowerSubexpression(evc, type, positive_cxt);
    }
    auto fresh = FindFreshEVar(evar);
    if (fresh.is_fresh && !positive_cxt) {
      negated_evars_.insert(evar);
    }
    absl::StrAppend(&code_, "v", fresh.id);
    return true;
  } else {
    LOG(ERROR) << "unknown subexpression kind";
    return false;
  }
}

bool SouffleProgram::AssignEVarType(EVar* evar, EVarType type) {
  auto t = evar_types_.insert({evar, type});
  if (t.second) return true;
  return t.first->second == type;
}

bool SouffleProgram::AssignEVarType(EVar* evar, EVar* oevar) {
  auto t = evar_types_.find(evar);
  if (t == evar_types_.end()) {
    auto s = evar_types_.find(oevar);
    if (s != evar_types_.end()) return AssignEVarType(evar, s->second);
    // A fancier implementation would keep track of these relations.
    return true;
  } else {
    return AssignEVarType(oevar, t->second);
  }
}

bool SouffleProgram::LowerGoalGroup(const SymbolTable& symbol_table,
                                    const GoalGroup& group, int target_goal) {
#ifdef DEBUG_LOWERING
  FileHandlePrettyPrinter dprinter(stderr);
  for (const auto& goal : group.goals) {
    dprinter.Print("goal <");
    goal->Dump(symbol_table, &dprinter);
    dprinter.Print(">\n");
  }
  size_t ccode = code_.size();
#endif
  if (group.goals.empty()) return true;
  bool pos = group.accept_if != GoalGroup::AcceptanceCriterion::kSomeMustFail;
  int cur_goal = 0;
  if (pos) {
    absl::StrAppend(&code_, ", (true");
    for (const auto& goal : group.goals) {
      if (target_goal >= 0 && cur_goal > target_goal) break;
      ++cur_goal;
      if (!LowerGoal(symbol_table, goal, true)) return false;
    }
    absl::StrAppend(&code_, ")\n");
  } else {
    absl::StrAppend(&code_, ", 0 = count:{true, sym(_)");
    for (const auto& goal : group.goals) {
      if (target_goal >= 0 && cur_goal > target_goal) break;
      ++cur_goal;
      if (!LowerGoal(symbol_table, goal, false)) return false;
    }
    absl::StrAppend(&code_, "}\n");
  }
#ifdef DEBUG_LOWERING
  dprinter.Print(" => <");
  dprinter.Print(code_.substr(ccode, code_.size() - ccode));
  dprinter.Print("> \n");
#endif
  return true;
}

bool SouffleProgram::LowerGoal(const SymbolTable& symbol_table, AstNode* goal,
                               bool positive_cxt) {
  auto eq_sym = symbol_table.MustIntern("=");
  auto empty_sym = symbol_table.MustIntern("");
  auto* app = goal->AsApp();
  auto* tup = app->rhs()->AsTuple();
  if (app->lhs()->AsIdentifier()->symbol() == eq_sym) {
    auto EqEvarRange = [&](EVar* evar, Range* range) {
      auto beginsym = symbol_table.FindInterned(std::to_string(range->begin()));
      auto endsym = symbol_table.FindInterned(std::to_string(range->end()));
      if (!beginsym || !endsym) {
        // TODO(zarko): emit a warning here (if we're in a positive goal)?
        absl::StrAppend(&code_, ", false");
      } else {
        auto fresh = FindFreshEVar(evar);
        if (fresh.is_fresh && !positive_cxt) {
          negated_evars_.insert(evar);
        }
        // We need to name the elements of the range; otherwise the compiler
        // will complain that they are ungrounded in certain cases.
        absl::StrAppend(&code_, ", v", fresh.id, "=[v", fresh.id, "r1, ",
                        range->corpus(), ", ", range->root(), ", ",
                        range->path(), ", v", fresh.id, "r2]");
        // TODO(zarko): there might be a cleaner way to handle eq_sym; it would
        // need LowerSubexpression to be able to emit this as a side-clause.
        absl::StrAppend(&code_, ", at(", *beginsym, ", ", *endsym, ", v",
                        fresh.id, ")");
      }
      return AssignEVarType(evar, EVarType::kVName);
    };
    auto EqEvarSubexp = [&](EVar* evar, AstNode* subexp) {
      auto fresh = FindFreshEVar(evar);
      if (fresh.is_fresh && !positive_cxt) {
        negated_evars_.insert(evar);
      }
      absl::StrAppend(&code_, ", v", fresh.id, "=");
      if (auto* app = subexp->AsApp()) {
        AssignEVarType(evar, EVarType::kVName);
        return LowerSubexpression(app, EVarType::kVName, positive_cxt);
      } else if (auto* ident = subexp->AsIdentifier()) {
        AssignEVarType(evar, EVarType::kSymbol);
        return LowerSubexpression(ident, EVarType::kSymbol, positive_cxt);
      } else if (auto* oevar = subexp->AsEVar()) {
        auto ofresh = FindFreshEVar(oevar);
        if (ofresh.is_fresh && !positive_cxt) {
          negated_evars_.insert(oevar);
        }
        absl::StrAppend(&code_, "v", ofresh.id);
        return AssignEVarType(evar, oevar);
      } else {
        LOG(ERROR) << "expected equality on evar, app, ident, or range";
        return false;
      }
    };
    auto* revar = tup->element(1)->AsEVar();
    auto* rrange = tup->element(1)->AsRange();
    auto* levar = tup->element(0)->AsEVar();
    auto* lrange = tup->element(0)->AsRange();
    if (revar != nullptr && lrange != nullptr) {
      return EqEvarRange(revar, lrange);
    } else if (levar != nullptr && lrange != nullptr) {
      return EqEvarRange(levar, rrange);
    } else if (levar != nullptr) {
      return EqEvarSubexp(levar, tup->element(1));
    } else if (revar != nullptr) {
      return EqEvarSubexp(revar, tup->element(0));
    } else {
      LOG(ERROR) << "expected eqality with evar on some side";
      return false;
    }
  } else {
    // This is an edge or fact pattern.
    absl::StrAppend(&code_, ", entry(");
    for (size_t p = 0; p < 5; ++p) {
      if (p != 0) {
        absl::StrAppend(&code_, ", ");
      }
      if (p == 2 && tup->element(p)->AsIdentifier() &&
          tup->element(p)->AsIdentifier()->symbol() == empty_sym) {
        // Facts have nil vnames in the target position.
        absl::StrAppend(&code_, "nil");
        continue;
      }
      if (!LowerSubexpression(
              tup->element(p),
              p == 0 || p == 2 ? EVarType::kVName : EVarType::kSymbol,
              positive_cxt)) {
        return false;
      }
    }
    absl::StrAppend(&code_, ")");
  }
  return true;
}

bool SouffleProgram::Lower(const SymbolTable& symbol_table,
                           const std::vector<GoalGroup>& goal_groups,
                           const std::vector<Inspection>& inspections,
                           const SouffleErrorState& error_state) {
  code_.clear();
  int cur_group = 0;
  for (const auto& group : goal_groups) {
    if (error_state.IsFinished(cur_group)) break;
    if (!LowerGoalGroup(symbol_table, group,
                        error_state.GoalForGroup(cur_group)))
      return false;
    ++cur_group;
  }
  if (emit_prelude_) {
    std::string code;
    code_.swap(code);
    std::string result_tyspec;  // ".type result_ty = ..." or ""
    std::string result_spec;    // .decl result($result_spec)
    std::string
        result_argspec;  // result($result_argspec) :- true$result_clause
    std::string result_clause;
    absl::flat_hash_set<size_t> inspected_vars;
    for (const auto& i : inspections) {
      if (negated_evars_.contains(i.evar)) {
        // TODO(zarko): If we intend to preserve this restriction, it would be
        // better to catch it earlier (possibly during goal parsing). It's
        // possible to support it (e.g., an evar in a negative context with an
        // inspection will be inspected if the negative context fails, giving a
        // witness for *why* that negative goal group failed), but this will
        // complicate error recovery. This message is still better than getting
        // a diagnostic from Souffle about a leaky witness.
        if (i.kind == Inspection::Kind::IMPLICIT) {
          // Ignore implicit inspections inside negated contexts.
          continue;
        }
        LOG(ERROR) << i.evar->location() << ": " << i.label
                   << ": can't inspect a negated evar";
        return false;
      }
      auto type = evar_types_.find(i.evar);
      if (type == evar_types_.end()) {
        LOG(ERROR) << "evar typing missing for v" << FindEVar(i.evar);
        return false;
      }
      if (type->second == EVarType::kUnknown) {
        LOG(ERROR) << "evar typing unknown for v" << FindEVar(i.evar);
        return false;
      }
      auto id = FindEVar(i.evar);
      if (inspected_vars.insert(id).second) {
        if (result_spec.empty()) {
          result_spec = "rrec : result_ty";
          result_argspec = "rrec";
        }
        absl::StrAppend(&result_clause,
                        result_clause.empty() ? ", rrec=[" : ", ", "v", id);
        absl::StrAppend(&result_tyspec,
                        result_tyspec.empty() ? ".type result_ty = [" : ", ",
                        "rv", id, ":",
                        type->second == EVarType::kVName ? "vname" : "number");

        inspections_.push_back(i.evar);
      }
    }
    if (!result_spec.empty()) {
      // result_ty has been defined and will always be a record with at least
      // one element; we need to terminate both the type definition and the
      // record expression.
      absl::StrAppend(&result_tyspec, "]");
      absl::StrAppend(&result_clause, "]");
    }
    code_ = absl::Substitute(kGlobalDecls, symbol_table.MustIntern(""),
                             symbol_table.MustIntern("/kythe/node/kind"),
                             symbol_table.MustIntern("anchor"),
                             symbol_table.MustIntern("/kythe/loc/start"),
                             symbol_table.MustIntern("/kythe/loc/end"),
                             result_tyspec, result_spec, result_argspec,
                             result_clause);
    absl::StrAppend(&code_, code);
  }
  absl::StrAppend(&code_, ".\n");
#ifdef DEBUG_LOWERING
  fprintf(stderr, "<%s>\n", code_.c_str());
#endif
  return true;
}
}  // namespace kythe::verifier
