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
.decl result()
result() :- true
)";
}

bool SouffleProgram::LowerSubexpression(AstNode* node) {
  if (auto* app = node->AsApp()) {
    auto* tup = app->rhs()->AsTuple();
    absl::StrAppend(&code_, "[");
    for (size_t p = 0; p < 5; ++p) {
      if (p != 0) {
        absl::StrAppend(&code_, ", ");
      }
      if (!LowerSubexpression(tup->element(p))) {
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
      return LowerSubexpression(evc);
    }
    absl::StrAppend(&code_, "v", FindEVar(evar));
    return true;
  } else {
    LOG(ERROR) << "unknown subexpression kind";
    return false;
  }
}

bool SouffleProgram::LowerGoalGroup(const SymbolTable& symbol_table,
                                    const GoalGroup& group) {
  auto eq_sym = symbol_table.MustIntern("=");
  auto empty_sym = symbol_table.MustIntern("");
#ifdef DEBUG_LOWERING
  FileHandlePrettyPrinter dprinter(stderr);
#endif
  for (const auto& goal : group.goals) {
#ifdef DEBUG_LOWERING
    dprinter.Print("goal <");
    goal->Dump(symbol_table, &dprinter);
    dprinter.Print(">\n");
    size_t ccode = code_.size();
#endif
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
          absl::StrAppend(&code_, ", v", FindEVar(evar), "=[_, ",
                          range->corpus(), ", ", range->path(), ", ",
                          range->root(), ", _]");
          absl::StrAppend(&code_, ", at(", *beginsym, ", ", *endsym, ", v",
                          FindEVar(evar), ")");
        }
      } else {
        auto* oevar = tup->element(0)->AsEVar();
        if (oevar == nullptr) {
          LOG(ERROR) << "expected evar or range on lhs of eq";
          return false;
        }
        absl::StrAppend(&code_, ", v", FindEVar(evar), "=v", FindEVar(oevar));
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
        if (!LowerSubexpression(tup->element(p))) {
          return false;
        }
      }
      absl::StrAppend(&code_, ")");
    }
#ifdef DEBUG_LOWERING
    dprinter.Print(" => <");
    dprinter.Print(code_.substr(ccode, code_.size() - ccode));
    dprinter.Print("> \n");
#endif
  }
  absl::StrAppend(&code_, ".\n");
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
  CHECK_LE(goal_groups.size(), 1) << "(unimplemented)";
  if (goal_groups.size() == 0) {
    absl::StrAppend(&code_, ".\n");
    return true;
  }
  for (const auto& group : goal_groups) {
    if (!LowerGoalGroup(symbol_table, group)) return false;
  }
  return true;
}
}  // namespace kythe::verifier
