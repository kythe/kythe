/*
 * Copyright 2014 Google Inc. All rights reserved.
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

#include "assertions.h"

#include <sstream>

#include "verifier.h"

namespace kythe {
namespace verifier {

void EVar::Dump(const SymbolTable &symbol_table, PrettyPrinter *printer) {
  printer->Print("EVar(");
  printer->Print(this);
  printer->Print(" = ");
  if (AstNode *node = current()) {
    node->Dump(symbol_table, printer);
  } else {
    printer->Print("<nullptr>");
  }
  printer->Print(")");
}

void Identifier::Dump(const SymbolTable &symbol_table, PrettyPrinter *printer) {
  const auto &text = symbol_table.text(symbol_);
  if (text.size()) {
    printer->Print(text);
  } else {
    printer->Print("\"\"");
  }
}

void Tuple::Dump(const SymbolTable &symbol_table, PrettyPrinter *printer) {
  printer->Print("(");
  for (size_t v = 0; v < element_count_; ++v) {
    elements_[v]->Dump(symbol_table, printer);
    if (v + 1 < element_count_) {
      printer->Print(", ");
    }
  }
  printer->Print(")");
}

void App::Dump(const SymbolTable &symbol_table, PrettyPrinter *printer) {
  printer->Print("App(");
  lhs_->Dump(symbol_table, printer);
  printer->Print(", ");
  rhs_->Dump(symbol_table, printer);
  printer->Print(")");
}

bool AssertionParser::ParseInlineRuleString(const std::string &content,
                                            const std::string &fake_filename,
                                            const char *comment_prefix) {
  lex_check_against_ = comment_prefix;
  had_errors_ = false;
  files_.push_back(fake_filename);
  ResetLexCheck();
  ScanBeginString(content, trace_lex_);
  yy::AssertionParserImpl parser(*this);
  parser.set_debug_level(trace_parse_);
  int result = parser.parse();
  ScanEnd(last_eof_, last_eof_ofs_);
  return result == 0 && !had_errors_;
}

bool AssertionParser::ParseInlineRuleFile(const std::string &filename,
                                          const char *comment_prefix) {
  lex_check_against_ = comment_prefix;
  files_.push_back(filename);
  had_errors_ = false;
  ResetLexCheck();
  ScanBeginFile(trace_lex_);
  yy::AssertionParserImpl parser(*this);
  parser.set_debug_level(trace_parse_);
  int result = parser.parse();
  ScanEnd(last_eof_, last_eof_ofs_);
  return result == 0 && !had_errors_;
}

void AssertionParser::Error(const yy::location &location,
                            const std::string &message) {
  // TODO(zarko): replace with a PrettyPrinter
  std::cerr << location << ": " << message << std::endl;
  had_errors_ = true;
}

void AssertionParser::Error(const std::string &message) {
  // TODO(zarko): replace with a PrettyPrinter
  std::cerr << "When trying " << file() << ": " << message << std::endl;
  had_errors_ = true;
}

AssertionParser::AssertionParser(Verifier *verifier, bool trace_lex,
                                 bool trace_parse)
    : verifier_(*verifier),
      arena_(verifier->arena()),
      trace_lex_(trace_lex),
      trace_parse_(trace_parse) {
  groups_.push_back(GoalGroup{GoalGroup::kNoneMayFail});
}

bool AssertionParser::Unescape(const char *yytext, std::string *out) {
  if (out == nullptr || *yytext != '\"') {
    return false;
  }
  ++yytext;  // Skip initial ".
  out->clear();
  char current = *yytext++;  // yytext will always immediately follow `current`.
  for (; current != '\0' && current != '\"'; current = *yytext++) {
    if (current == '\\') {
      current = *yytext++;
      switch (current) {
        case '\"':
          out->push_back(current);
          break;
        case '\\':
          out->push_back(current);
          break;
        case 'n':
          out->push_back('\n');
          break;
        default:
          return false;
      }
    } else {
      out->push_back(current);
    }
  }
  return (current == '\"' && *yytext == '\0');
}

void AssertionParser::ResetLexCheck() {
  lex_check_buffer_size_ = 0;
  line_.clear();
}

void AssertionParser::PushLocationSpec(const std::string &for_token) {
  location_spec_stack_.push_back(for_token);
}

Identifier *AssertionParser::PathIdentifierFor(
    const yy::location &location, const std::string &path_frag,
    const std::string &default_root) {
  if (path_frag.size() == 0) {
    return verifier_.IdentifierFor(location, "/");
  } else if (path_frag[0] != '/') {
    return verifier_.IdentifierFor(location, default_root + path_frag);
  }
  return verifier_.IdentifierFor(location, path_frag);
}

AstNode *AssertionParser::CreateEqualityConstraint(const yy::location &location,
                                                   AstNode *lhs, AstNode *rhs) {
  return verifier_.MakePredicate(location, verifier_.eq_id(), {lhs, rhs});
}

AstNode *AssertionParser::CreateSimpleEdgeFact(const yy::location &location,
                                               AstNode *edge_lhs,
                                               const std::string &literal_kind,
                                               AstNode *edge_rhs,
                                               AstNode *ordinal) {
  if (ordinal) {
    return verifier_.MakePredicate(
        location, verifier_.fact_id(),
        {edge_lhs, PathIdentifierFor(location, literal_kind, "/kythe/edge/"),
         edge_rhs, verifier_.ordinal_id(), ordinal});
  } else {
    return verifier_.MakePredicate(
        location, verifier_.fact_id(),
        {edge_lhs, PathIdentifierFor(location, literal_kind, "/kythe/edge/"),
         edge_rhs, verifier_.root_id(), verifier_.empty_string_id()});
  }
}

AstNode *AssertionParser::CreateSimpleNodeFact(const yy::location &location,
                                               AstNode *lhs,
                                               const std::string &literal_key,
                                               AstNode *value) {
  return verifier_.MakePredicate(
      location, verifier_.fact_id(),
      {lhs, verifier_.empty_string_id(), verifier_.empty_string_id(),
       PathIdentifierFor(location, literal_key, "/kythe/"), value});
}

AstNode *AssertionParser::CreateInspect(const yy::location &location,
                                        const std::string &inspect_id,
                                        AstNode *to_inspect) {
  if (EVar *evar = to_inspect->AsEVar()) {
    inspections_.emplace_back(inspect_id, evar);
    return to_inspect;
  } else {
    Error(location, "Inspecting something that's not an EVar.");
    return to_inspect;
  }
}

AstNode *AssertionParser::CreateDontCare(const yy::location &location) {
  return new (verifier_.arena()) EVar(location);
}

AstNode *AssertionParser::CreateAtom(const yy::location &location,
                                     const std::string &for_token) {
  if (for_token.size() && isupper(for_token[0])) {
    return CreateEVar(location, for_token);
  } else {
    return CreateIdentifier(location, for_token);
  }
}

Identifier *AssertionParser::CreateIdentifier(const yy::location &location,
                                              const std::string &for_text) {
  Symbol symbol = verifier_.symbol_table()->intern(for_text);
  const auto old_binding = identifier_context_.find(symbol);
  if (old_binding == identifier_context_.end()) {
    Identifier *new_id = new (verifier_.arena()) Identifier(location, symbol);
    identifier_context_.emplace(symbol, new_id);
    return new_id;
  } else {
    return old_binding->second;
  }
}

EVar *AssertionParser::CreateEVar(const yy::location &location,
                                  const std::string &for_token) {
  Symbol symbol = verifier_.symbol_table()->intern(for_token);
  const auto old_binding = evar_context_.find(symbol);
  if (old_binding == evar_context_.end()) {
    EVar *new_evar = new (verifier_.arena()) EVar(location);
    evar_context_.emplace(symbol, new_evar);
    return new_evar;
  } else {
    return old_binding->second;
  }
}

AstNode *AssertionParser::CreateAnchorSpec(const yy::location &location) {
  if (!location_spec_stack_.size()) {
    Error(location, "No locations on location stack.");
    return verifier_.empty_string_id();
  }
  EVar *new_evar = new (verifier_.arena()) EVar(location);
  unresolved_locations_.push_back(
      UnresolvedLocation{new_evar, location_spec_stack_.back(), group_id(),
                         UnresolvedLocation::Kind::kAnchor});
  location_spec_stack_.pop_back();
  AppendGoal(group_id(), verifier_.MakePredicate(
                             location, verifier_.fact_id(),
                             {new_evar, verifier_.empty_string_id(),
                              verifier_.empty_string_id(), verifier_.kind_id(),
                              verifier_.IdentifierFor(location, "anchor")}));
  return new_evar;
}

AstNode *AssertionParser::CreateOffsetSpec(const yy::location &location,
                                           bool at_end) {
  if (!location_spec_stack_.size()) {
    Error(location, "No locations on location stack.");
    return verifier_.empty_string_id();
  }
  EVar *new_evar = new (verifier_.arena()) EVar(location);
  unresolved_locations_.push_back(
      UnresolvedLocation{new_evar, location_spec_stack_.back(), group_id(),
                         at_end ? UnresolvedLocation::Kind::kOffsetEnd
                                : UnresolvedLocation::Kind::kOffsetBegin});
  location_spec_stack_.pop_back();
  return new_evar;
}

bool AssertionParser::ResolveLocations(const yy::location &end_of_line,
                                       size_t offset_after_endline) {
  bool was_ok = true;
  for (auto &record : unresolved_locations_) {
    size_t group_id = record.group_id;
    EVar *evar = record.anchor_evar;
    std::string &token = record.anchor_text;
    yy::location location = evar->location();
    auto col = line_.find(token);
    if (col == std::string::npos) {
      Error(location, token + " not found.");
      was_ok = false;
      continue;
    }
    if (line_.find(token, col + 1) != std::string::npos) {
      Error(location, token + " is ambiguous.");
      was_ok = false;
      continue;
    }
    size_t line_start = offset_after_endline - line_.size() - 1;
    switch (record.kind) {
      case UnresolvedLocation::Kind::kOffsetBegin:
        if (evar->current()) {
          Error(location, token + " already resolved.");
          was_ok = false;
          continue;
        }
        evar->set_current(verifier_.IdentifierFor(
            location, std::to_string(line_start + col)));
        break;
      case UnresolvedLocation::Kind::kOffsetEnd:
        if (evar->current()) {
          Error(location, token + " already resolved.");
          was_ok = false;
          continue;
        }
        evar->set_current(verifier_.IdentifierFor(
            location, std::to_string(line_start + col + token.size())));
        break;
      case UnresolvedLocation::Kind::kAnchor:
        AppendGoal(
            group_id,
            verifier_.MakePredicate(
                location, verifier_.fact_id(),
                {evar, verifier_.empty_string_id(), verifier_.empty_string_id(),
                 verifier_.IdentifierFor(location, "/kythe/loc/start"),
                 verifier_.IdentifierFor(location, line_start + col)}));
        AppendGoal(
            group_id,
            verifier_.MakePredicate(
                location, verifier_.fact_id(),
                {evar, verifier_.empty_string_id(), verifier_.empty_string_id(),
                 verifier_.IdentifierFor(location, "/kythe/loc/end"),
                 verifier_.IdentifierFor(location,
                                         line_start + col + token.size())}));
        break;
    }
  }
  unresolved_locations_.clear();
  ResetLexCheck();
  return was_ok;
}

void AssertionParser::AppendToLine(const char *yytext) { line_.append(yytext); }

int AssertionParser::NextLexCheck(const char *yytext) {
  char ch = yytext[0];
  size_t max_len = lex_check_against_.size();
  if (max_len == 0) {
    return 1;
  }
  line_.push_back(ch);
  if (lex_check_buffer_size_ == 0 && (ch == '\t' || ch == ' ')) {
    return 0;
  } else if (ch == lex_check_against_[lex_check_buffer_size_++]) {
    return static_cast<size_t>(lex_check_buffer_size_) == max_len ? 1 : 0;
  }
  return -1;
}

void AssertionParser::PushNode(AstNode *node) { node_stack_.push_back(node); }

AstNode **AssertionParser::PopNodes(size_t count) {
  AstNode **nodes =
      (AstNode **)verifier_.arena()->New(count * sizeof(AstNode *));
  size_t start = node_stack_.size() - count;
  for (size_t c = 0; c < count; ++c) {
    nodes[c] = node_stack_[start + c];
  }
  node_stack_.resize(start);
  return nodes;
}

void AssertionParser::AppendGoal(size_t group_id, AstNode *goal) {
  assert(group_id < groups_.size());
  groups_[group_id].goals.push_back(goal);
}

void AssertionParser::EnterGoalGroup(const yy::location &location,
                                     bool negated) {
  if (inside_goal_group_) {
    Error(location, "It is not valid to enter nested goal groups.");
    return;
  }
  inside_goal_group_ = true;
  groups_.push_back(
      GoalGroup{negated ? GoalGroup::kSomeMustFail : GoalGroup::kNoneMayFail});
}

void AssertionParser::ExitGoalGroup(const yy::location &location) {
  if (!inside_goal_group_) {
    Error(location, "You've left a goal group before you've entered it.");
    return;
  }
  inside_goal_group_ = false;
}

}  // namespace verifier
}  // namespace kythe
