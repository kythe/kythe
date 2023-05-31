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

#include "absl/strings/substitute.h"

namespace kythe::verifier {
namespace {
constexpr absl::string_view kGlobalDecls = R"(
.type vname = [
  signature:number,
  corpus:number,
  path:number,
  root:number,
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
.decl gvn(vname:vname)
gvn(v) :- !(v = nil), (entry(v, _, _, _, _); entry(_, _, v, _, _)).
.decl gsym(s:number)
gsym(s) :- entry(_, s, _, _, _); entry(_, _, _, s, _); entry(_, _, _, _, s).
.decl result()
result() :- true
)";
}

// #define DEBUG_LOWERING

bool SouffleProgram::LowerSubexpression(bool pos, EVarType type,
                                        AstNode* node) {
  if (auto* app = node->AsApp()) {
    auto* tup = app->rhs()->AsTuple();
    absl::StrAppend(&code_, "[");
    for (size_t p = 0; p < 5; ++p) {
      if (p != 0) {
        absl::StrAppend(&code_, ", ");
      }
      if (!LowerSubexpression(pos, EVarType::kSymbol, tup->element(p))) {
        return false;
      }
    }
    absl::StrAppend(&code_, "]");
    return true;
  } else if (auto* id = node->AsIdentifier()) {
    absl::StrAppend(&code_, id->symbol());
    return true;
  } else if (auto* evar = node->AsEVar()) {
    if (auto* evc = evar->current()) {
      return LowerSubexpression(pos, type, evc);
    }
    absl::StrAppend(&code_, "v", FindEVar(evar, pos, type));
    return true;
  } else {
    LOG(ERROR) << "unknown subexpression kind";
    return false;
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
      if (!LowerGoal(symbol_table, pos, goal)) return false;
    }
    absl::StrAppend(&code_, ")\n");
  } else {
    /*absl::StrAppend(&code_, ", (false");
    for (size_t max = 0; max < group.goals.size(); ++max) {
      absl::StrAppend(&code_, "; (true");
      for (size_t min = 0; min < max; ++min) {
        if (!LowerGoal(symbol_table, true, group.goals[min])) return false;
      }
      if (!LowerGoal(symbol_table, false, group.goals[max])) return false;
      absl::StrAppend(&code_, ")");
    }
    absl::StrAppend(&code_, ")\n");*/
    absl::StrAppend(&code_, ", 0 = count:{true");
    for (const auto& goal : group.goals) {
      if (!LowerGoal(symbol_table, true, goal)) return false;
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

bool SouffleProgram::LowerGoal(const SymbolTable& symbol_table,
                               bool pos,
                               AstNode* goal) {
  auto eq_sym = symbol_table.MustIntern("=");
  auto empty_sym = symbol_table.MustIntern("");
  // absl::StrAppend(&code_, pos ? ", (true" : ", !(true");
  auto* app = goal->AsApp();
  auto* tup = app->rhs()->AsTuple();
  if (app->lhs()->AsIdentifier()->symbol() == eq_sym) {
    auto* evar = tup->element(1)->AsEVar();
    if (evar == nullptr) {
      LOG(ERROR) << "expected evar on rhs of eq";
      return false;
    }
    if (auto* range = tup->element(0)->AsRange()) {
      auto beginsym =
          symbol_table.FindInterned(std::to_string(range->begin()));
      auto endsym = symbol_table.FindInterned(std::to_string(range->end()));
      if (!beginsym || !endsym) {
        // TODO(zarko): emit a warning here (if we're in a positive goal)?
        absl::StrAppend(&code_, ", false");
      } else {
        absl::StrAppend(&code_, ", v", FindEVar(evar, pos, EVarType::kVName),
                        "=[_, ", range->corpus(), ", ", range->path(), ", ",
                        range->root(), ", _]");
        absl::StrAppend(&code_, ", at(", *beginsym, ", ", *endsym, ", v",
                        FindEVar(evar, pos, EVarType::kVName), ")");
      }
    } else {
      auto* oevar = tup->element(0)->AsEVar();
      if (oevar == nullptr) {
        LOG(ERROR) << "expected evar or range on lhs of eq";
        return false;
      }
      absl::StrAppend(&code_, ", v", FindEVar(evar, pos, EVarType::kUnknown),
                      "=v", FindEVar(oevar, pos, EVarType::kUnknown));
      UnifyEVarTypes(evar, oevar);
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
              pos, p == 0 || p == 2 ? EVarType::kVName : EVarType::kSymbol,
              tup->element(p))) {
        return false;
      }
    }
    absl::StrAppend(&code_, ")");
  }
  // absl::StrAppend(&code_, ")\n");
  return true;
}

bool SouffleProgram::Lower(const SymbolTable& symbol_table,
                           const std::vector<GoalGroup>& goal_groups) {
  code_.clear();
  if (emit_prelude_) {
    code_ = absl::Substitute(kGlobalDecls, symbol_table.MustIntern(""),
                             symbol_table.MustIntern("/kythe/node/kind"),
                             symbol_table.MustIntern("anchor"),
                             symbol_table.MustIntern("/kythe/loc/start"),
                             symbol_table.MustIntern("/kythe/loc/end"));
  }
  for (const auto& group : goal_groups) {
    if (!LowerGoalGroup(symbol_table, group)) return false;
  }
  for (const auto& ev : evars_) {
    if (ev.second.appears_negative) {
      CHECK(ev.second.type != EVarType::kUnknown);
      absl::StrAppend(
          &code_, ev.second.type == EVarType::kVName ? ", gvn(v" : ", gsym(v",
          ev.second.index, ")");
    }
  }
  absl::StrAppend(&code_, ".\n");
#ifdef DEBUG_LOWERING
  fprintf(stderr, "<%s>\n", code_.c_str());
#endif
  return true;
}
}  // namespace kythe::verifier
