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
.decl entry(source:vname, kind:number, target:vname, name:number, value:number)
.input entry(IO=kythe)
.decl anchor(begin:number, end:number, vname:vname)
.input anchor(IO=kythe, anchors=1)
.decl at(startsym:number, endsym:number, vname:vname)
at(s, e, v) :- entry(v, $0, nil, $1, $2),
               entry(v, $0, nil, $3, s),
               entry(v, $0, nil, $4, e).
.decl result($5)
result($6) :- true
)";
}

bool SouffleProgram::LowerSubexpression(AstNode* node, EVarType type) {
  if (auto* app = node->AsApp()) {
    auto* tup = app->rhs()->AsTuple();
    absl::StrAppend(&code_, "[");
    for (size_t p = 0; p < 5; ++p) {
      if (p != 0) {
        absl::StrAppend(&code_, ", ");
      }
      if (!LowerSubexpression(tup->element(p), EVarType::kSymbol)) {
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
      return LowerSubexpression(evc, type);
    }
    absl::StrAppend(&code_, "v", FindEVar(evar));
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
                                    const GoalGroup& group) {
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
  if (pos) {
    absl::StrAppend(&code_, ", (true");
    for (const auto& goal : group.goals) {
      if (!LowerGoal(symbol_table, goal)) return false;
    }
    absl::StrAppend(&code_, ")\n");
  } else {
    absl::StrAppend(&code_, ", 0 = count:{true");
    for (const auto& goal : group.goals) {
      if (!LowerGoal(symbol_table, goal)) return false;
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

bool SouffleProgram::LowerGoal(const SymbolTable& symbol_table, AstNode* goal) {
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
        // We need to name the elements of the range; otherwise the compiler
        // will complain that they are ungrounded in certain cases.
        absl::StrAppend(&code_, ", v", FindEVar(evar), "=[v", FindEVar(evar),
                        "r1, ", range->corpus(), ", ", range->root(), ", ",
                        range->path(), ", v", FindEVar(evar), "r2]");
        // TODO(zarko): there might be a cleaner way to handle eq_sym; it would
        // need LowerSubexpression to be able to emit this as a side-clause.
        absl::StrAppend(&code_, ", at(", *beginsym, ", ", *endsym, ", v",
                        FindEVar(evar), ")");
      }
      return AssignEVarType(evar, EVarType::kVName);
    };
    auto EqEvarSubexp = [&](EVar* evar, AstNode* subexp) {
      absl::StrAppend(&code_, ", v", FindEVar(evar), "=");
      if (auto* app = subexp->AsApp()) {
        AssignEVarType(evar, EVarType::kVName);
        return LowerSubexpression(app, EVarType::kVName);
      } else if (auto* ident = subexp->AsIdentifier()) {
        AssignEVarType(evar, EVarType::kSymbol);
        return LowerSubexpression(ident, EVarType::kSymbol);
      } else if (auto* oevar = subexp->AsEVar()) {
        absl::StrAppend(&code_, "v", FindEVar(oevar));
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
      if (!LowerSubexpression(tup->element(p), p == 0 || p == 2
                                                   ? EVarType::kVName
                                                   : EVarType::kSymbol)) {
        return false;
      }
    }
    absl::StrAppend(&code_, ")");
  }
  return true;
}

bool SouffleProgram::Lower(const SymbolTable& symbol_table,
                           const std::vector<GoalGroup>& goal_groups,
                           const std::vector<Inspection>& inspections) {
  code_.clear();
  for (const auto& group : goal_groups) {
    if (!LowerGoalGroup(symbol_table, group)) return false;
  }
  if (emit_prelude_) {
    std::string code;
    code_.swap(code);
    std::string result_tyspec;
    std::string result_spec;
    absl::flat_hash_set<size_t> inspected_vars;
    for (const auto& i : inspections) {
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
        absl::StrAppend(&result_spec, result_spec.empty() ? "" : ", ", "v", id);
        absl::StrAppend(&result_tyspec, result_tyspec.empty() ? "" : ", ", "v",
                        id, ":",
                        type->second == EVarType::kVName ? "vname" : "number");
        inspections_.push_back(i.evar);
      }
    }
    code_ = absl::Substitute(kGlobalDecls, symbol_table.MustIntern(""),
                             symbol_table.MustIntern("/kythe/node/kind"),
                             symbol_table.MustIntern("anchor"),
                             symbol_table.MustIntern("/kythe/loc/start"),
                             symbol_table.MustIntern("/kythe/loc/end"),
                             result_tyspec, result_spec);
    absl::StrAppend(&code_, code);
  }
  absl::StrAppend(&code_, ".\n");
#ifdef DEBUG_LOWERING
  fprintf(stderr, "<%s>\n", code_.c_str());
#endif
  return true;
}
}  // namespace kythe::verifier
