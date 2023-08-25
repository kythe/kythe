/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "kythe/cxx/common/file_utils.h"
#include "verifier.h"

namespace kythe {
namespace verifier {

void EVar::Dump(const SymbolTable& symbol_table, PrettyPrinter* printer) {
  if (AstNode* node = current()) {
    node->Dump(symbol_table, printer);
  } else {
    printer->Print("<null>");
  }
}

void Identifier::Dump(const SymbolTable& symbol_table, PrettyPrinter* printer) {
  printer->Print(symbol_table.PrettyText(symbol_));
}

void Range::Dump(const SymbolTable& symbol_table, PrettyPrinter* printer) {
  printer->Print("Range(");
  printer->Print(symbol_table.PrettyText(corpus_));
  printer->Print(",");
  printer->Print(symbol_table.PrettyText(root_));
  printer->Print(",");
  printer->Print(symbol_table.PrettyText(path_));
  printer->Print(",");
  printer->Print(std::to_string(begin_));
  printer->Print(",");
  printer->Print(std::to_string(end_));
  printer->Print(")");
}

void Tuple::Dump(const SymbolTable& symbol_table, PrettyPrinter* printer) {
  printer->Print("(");
  for (size_t v = 0; v < element_count_; ++v) {
    elements_[v]->Dump(symbol_table, printer);
    if (v + 1 < element_count_) {
      printer->Print(", ");
    }
  }
  printer->Print(")");
}

void App::Dump(const SymbolTable& symbol_table, PrettyPrinter* printer) {
  lhs_->Dump(symbol_table, printer);
  // rhs_ should be a Tuple, which outputs "(...)" around itself.
  rhs_->Dump(symbol_table, printer);
}

bool AssertionParser::ParseInlineRuleString(const std::string& content,
                                            const std::string& fake_filename,
                                            Symbol path, Symbol root,
                                            Symbol corpus,
                                            const RE2& goal_comment_regex) {
  path_ = path;
  root_ = root;
  corpus_ = corpus;
  had_errors_ = false;
  files_.push_back(fake_filename);
  ResetLine();
  ScanBeginString(goal_comment_regex, content, trace_lex_);
  yy::AssertionParserImpl parser(*this);
  parser.set_debug_level(trace_parse_);
  int result = parser.parse();
  ScanEnd(last_eof_, last_eof_ofs_);
  return result == 0 && !had_errors_;
}

bool AssertionParser::ParseInlineRuleFile(const std::string& filename,
                                          Symbol path, Symbol root,
                                          Symbol corpus,
                                          const RE2& goal_comment_regex) {
  path_ = path;
  root_ = root;
  corpus_ = corpus;
  files_.push_back(filename);
  had_errors_ = false;
  ResetLine();
  ScanBeginFile(goal_comment_regex, trace_lex_);
  yy::AssertionParserImpl parser(*this);
  parser.set_debug_level(trace_parse_);
  int result = parser.parse();
  ScanEnd(last_eof_, last_eof_ofs_);
  return result == 0 && !had_errors_;
}

void AssertionParser::Error(const yy::location& location,
                            const std::string& message) {
  // TODO(zarko): replace with a PrettyPrinter
  std::cerr << location << ": " << message << std::endl;
  had_errors_ = true;
}

void AssertionParser::Error(const std::string& message) {
  // TODO(zarko): replace with a PrettyPrinter
  std::cerr << "When trying " << file() << ": " << message << std::endl;
  had_errors_ = true;
}

bool AssertionParser::CheckForSingletonEVars() {
  bool old_had_errors = had_errors_;
  for (const auto& singleton : singleton_evars_) {
    Error(singleton.first->location(),
          "singleton variable " +
              verifier_.symbol_table()->text(singleton.second) +
              " used only here");
  }
  had_errors_ = old_had_errors;
  return !singleton_evars_.empty();
}

AssertionParser::AssertionParser(Verifier* verifier, bool trace_lex,
                                 bool trace_parse)
    : verifier_(*verifier),
      arena_(verifier->arena()),
      trace_lex_(trace_lex),
      trace_parse_(trace_parse) {
  groups_.push_back(GoalGroup{GoalGroup::kNoneMayFail});
}

bool AssertionParser::Unescape(const char* yytext, std::string* out) {
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

void AssertionParser::ResetLine() { line_.clear(); }

void AssertionParser::PushLocationSpec(const std::string& for_token) {
  location_spec_stack_.emplace_back(LocationSpec{for_token, -1, false, true});
}

void AssertionParser::PushRelativeLocationSpec(const std::string& for_token,
                                               const std::string& relative) {
  location_spec_stack_.emplace_back(
      LocationSpec{for_token, atoi(relative.c_str()), false, true});
}

void AssertionParser::PushAbsoluteLocationSpec(const std::string& for_token,
                                               const std::string& absolute) {
  location_spec_stack_.emplace_back(
      LocationSpec{for_token, atoi(absolute.c_str()), true, true});
}

void AssertionParser::SetTopLocationSpecMatchNumber(const std::string& number) {
  if (!location_spec_stack_.empty()) {
    // number is "#"{blank}*{int}
    location_spec_stack_.back().must_be_unambiguous = false;
    location_spec_stack_.back().match_number = atoi(number.c_str() + 1);
  }
}

Identifier* AssertionParser::PathIdentifierFor(
    const yy::location& location, const std::string& path_frag,
    const std::string& default_root) {
  if (path_frag.empty()) {
    return verifier_.IdentifierFor(location, "/");
  }
  std::string sigil;
  if (path_frag[0] == '#' || path_frag[0] == '%') {
    sigil = path_frag[0];
    if (path_frag.size() == 1) {
      return verifier_.IdentifierFor(location, sigil);
    }
  }
  if (path_frag[sigil.size()] != '/') {
    return verifier_.IdentifierFor(
        location, sigil + default_root + path_frag.substr(sigil.size()));
  }
  return verifier_.IdentifierFor(location, path_frag);
}

AstNode* AssertionParser::CreateEqualityConstraint(const yy::location& location,
                                                   AstNode* lhs, AstNode* rhs) {
  return verifier_.MakePredicate(location, verifier_.eq_id(), {lhs, rhs});
}

AstNode* AssertionParser::CreateSimpleEdgeFact(const yy::location& location,
                                               AstNode* edge_lhs,
                                               const std::string& literal_kind,
                                               AstNode* edge_rhs,
                                               AstNode* ordinal) {
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

AstNode* AssertionParser::CreateSimpleNodeFact(const yy::location& location,
                                               AstNode* lhs,
                                               const std::string& literal_key,
                                               AstNode* value) {
  return verifier_.MakePredicate(
      location, verifier_.fact_id(),
      {lhs, verifier_.empty_string_id(), verifier_.empty_string_id(),
       PathIdentifierFor(location, literal_key, "/kythe/"), value});
}

AstNode* AssertionParser::CreateInspect(const yy::location& location,
                                        const std::string& inspect_id,
                                        AstNode* to_inspect) {
  if (EVar* evar = to_inspect->AsEVar()) {
    singleton_evars_.erase(evar);
    inspections_.emplace_back(inspect_id, evar, Inspection::Kind::EXPLICIT);
    return to_inspect;
  } else {
    Error(location, "Inspecting something that's not an EVar.");
    return to_inspect;
  }
}

AstNode* AssertionParser::CreateDontCare(const yy::location& location) {
  return new (verifier_.arena()) EVar(location);
}

AstNode* AssertionParser::CreateAtom(const yy::location& location,
                                     const std::string& for_token) {
  if (!for_token.empty() && for_token[0] == '_') {
    return CreateDontCare(location);
  } else if (!for_token.empty() && isupper(for_token[0])) {
    return CreateEVar(location, for_token);
  } else {
    return CreateIdentifier(location, for_token);
  }
}

Identifier* AssertionParser::CreateIdentifier(const yy::location& location,
                                              const std::string& for_text) {
  Symbol symbol = verifier_.symbol_table()->intern(for_text);
  const auto old_binding = identifier_context_.find(symbol);
  if (old_binding == identifier_context_.end()) {
    Identifier* new_id = new (verifier_.arena()) Identifier(location, symbol);
    identifier_context_.emplace(symbol, new_id);
    return new_id;
  } else {
    return old_binding->second;
  }
}

EVar* AssertionParser::CreateEVar(const yy::location& location,
                                  const std::string& for_token) {
  Symbol symbol = verifier_.symbol_table()->intern(for_token);
  const auto old_binding = evar_context_.find(symbol);
  if (old_binding == evar_context_.end()) {
    EVar* new_evar = new (verifier_.arena()) EVar(location);
    evar_context_.emplace(symbol, new_evar);
    if (default_inspect_) {
      inspections_.emplace_back(for_token, new_evar,
                                Inspection::Kind::IMPLICIT);
    }
    singleton_evars_[new_evar] = symbol;
    return new_evar;
  } else {
    singleton_evars_.erase(old_binding->second);
    return old_binding->second;
  }
}

bool AssertionParser::ValidateTopLocationSpec(const yy::location& location,
                                              size_t* line_number,
                                              bool* use_line_number,
                                              bool* must_be_unambiguous,
                                              int* match_number) {
  if (location_spec_stack_.empty()) {
    Error(location, "No locations on location stack.");
    return verifier_.empty_string_id();
  }
  const auto& spec = location_spec_stack_.back();
  *must_be_unambiguous = spec.must_be_unambiguous;
  *match_number = spec.match_number;
  if (spec.line_offset == 0) {
    Error(location, "This line offset is invalid.");
    return verifier_.empty_string_id();
  } else if (spec.line_offset < 0) {
    *use_line_number = false;
    *line_number = 0;
    return true;
  }
  *use_line_number = true;
  *line_number = spec.is_absolute ? spec.line_offset
                                  : spec.line_offset + location.begin.line;
  if (*line_number <= location.begin.line) {
    Error(location, "This line offset points to a previous or equal line.");
    return false;
  }
  return true;
}

AstNode* AssertionParser::CreateAnchorSpec(const yy::location& location) {
  size_t line_number = -1;
  bool use_line_number = false;
  bool must_be_unambiguous = false;
  int match_number = -1;
  if (!ValidateTopLocationSpec(location, &line_number, &use_line_number,
                               &must_be_unambiguous, &match_number)) {
    return verifier_.empty_string_id();
  }
  const auto& spec = location_spec_stack_.back();
  EVar* new_evar = new (verifier_.arena()) EVar(location);
  unresolved_locations_.push_back(UnresolvedLocation{
      new_evar, spec.spec, line_number, use_line_number, group_id(),
      UnresolvedLocation::Kind::kAnchor, must_be_unambiguous, match_number});
  location_spec_stack_.pop_back();
  return new_evar;
}

AstNode* AssertionParser::CreateOffsetSpec(const yy::location& location,
                                           bool at_end) {
  size_t line_number = -1;
  bool use_line_number = false;
  bool must_be_unambiguous = false;
  int match_number = -1;
  if (!ValidateTopLocationSpec(location, &line_number, &use_line_number,
                               &must_be_unambiguous, &match_number)) {
    return verifier_.empty_string_id();
  }
  const auto& spec = location_spec_stack_.back();
  EVar* new_evar = new (verifier_.arena()) EVar(location);
  unresolved_locations_.push_back(UnresolvedLocation{
      new_evar, spec.spec, line_number, use_line_number, group_id(),
      at_end ? UnresolvedLocation::Kind::kOffsetEnd
             : UnresolvedLocation::Kind::kOffsetBegin,
      must_be_unambiguous, match_number});
  location_spec_stack_.pop_back();
  return new_evar;
}

bool AssertionParser::ResolveLocations(const yy::location& end_of_line,
                                       size_t offset_after_endline,
                                       bool end_of_file) {
  bool was_ok = true;
  std::vector<UnresolvedLocation> succ_lines;
  for (auto& record : unresolved_locations_) {
    EVar* evar = record.anchor_evar;
    std::string& token = record.anchor_text;
    yy::location location = evar->location();
    location.columns(token.size());
    if (record.use_line_number &&
        (record.line_number != end_of_line.begin.line)) {
      if (end_of_file) {
        Error(location, token + ":" + std::to_string(record.line_number) +
                            " not found before end of file.");
        was_ok = false;
      } else {
        succ_lines.push_back(record);
      }
      continue;
    }
    size_t group_id = record.group_id;
    auto col = line_.find(token);
    if (col == std::string::npos) {
      Error(location, token + " not found.");
      was_ok = false;
      continue;
    }
    if (record.must_be_unambiguous) {
      if (line_.find(token, col + 1) != std::string::npos) {
        Error(location, token + " is ambiguous.");
        was_ok = false;
        continue;
      }
    } else {
      int match_number = 0;
      while (match_number != record.match_number) {
        col = line_.find(token, col + 1);
        if (col == std::string::npos) {
          break;
        }
        ++match_number;
      }
      if (match_number != record.match_number) {
        Error(location, token + " has no match #" +
                            std::to_string(record.match_number) + ".");
        was_ok = false;
        continue;
      }
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
        if (default_inspect_) {
          inspections_.emplace_back(
              absl::StrCat("@", token, ":", location.begin.line, ".", col),
              evar, Inspection::Kind::IMPLICIT);
        }
        AppendGoal(group_id, verifier_.MakePredicate(
                                 location, verifier_.eq_id(),
                                 {new (verifier_.arena())
                                      Range(location, line_start + col,
                                            line_start + col + token.size(),
                                            path_, root_, corpus_),
                                  evar}));
        break;
    }
  }
  unresolved_locations_.swap(succ_lines);
  ResetLine();
  return was_ok;
}

void AssertionParser::AppendToLine(const char* yytext) { line_.append(yytext); }

void AssertionParser::PushNode(AstNode* node) { node_stack_.push_back(node); }

AstNode** AssertionParser::PopNodes(size_t count) {
  AstNode** nodes = (AstNode**)verifier_.arena()->New(count * sizeof(AstNode*));
  size_t start = node_stack_.size() - count;
  for (size_t c = 0; c < count; ++c) {
    nodes[c] = node_stack_[start + c];
  }
  node_stack_.resize(start);
  return nodes;
}

void AssertionParser::AppendGoal(size_t group_id, AstNode* goal) {
  assert(group_id < groups_.size());
  groups_[group_id].goals.push_back(goal);
}

void AssertionParser::EnterGoalGroup(const yy::location& location,
                                     bool negated) {
  if (inside_goal_group_) {
    Error(location, "It is not valid to enter nested goal groups.");
    return;
  }
  inside_goal_group_ = true;
  groups_.push_back(
      GoalGroup{negated ? GoalGroup::kSomeMustFail : GoalGroup::kNoneMayFail});
}

void AssertionParser::ExitGoalGroup(const yy::location& location) {
  if (!inside_goal_group_) {
    Error(location, "You've left a goal group before you've entered it.");
    return;
  }
  inside_goal_group_ = false;
}

void AssertionParser::ScanBeginString(const RE2& goal_comment_regex,
                                      const std::string& data,
                                      bool trace_scanning) {
  // Preprocess the input by adding a - to the left of every goal line and a
  // . to the left of every non-goal line. From every goal line remove any
  // character that is not part of the goal regex's capture group. This means
  // that we don't have to push RE2 deeper into the lexer; it also preserves
  // file locations for diagnostics (after taking into account the constant
  // 1 offset).
  std::string yy_buf;
  size_t next_line_begin = 0;
  auto append_line = [&](size_t line_end) {
    absl::string_view match_region;
    size_t line_length = line_end - next_line_begin;
    auto is_goal = RE2::FullMatch(
        absl::string_view(data.data() + next_line_begin, line_length),
        goal_comment_regex, &match_region);
    if (is_goal == 1) {
      yy_buf.push_back('-');
      size_t pre_pad = match_region.data() - data.data() - next_line_begin;
      for (size_t s = 0; s < pre_pad; ++s) {
        yy_buf.push_back(' ');
      }
      yy_buf.append(match_region.data(), match_region.size());
      size_t post_pad = line_length - pre_pad - match_region.size();
      for (size_t s = 0; s < post_pad; ++s) {
        yy_buf.push_back(' ');
      }
    } else {
      yy_buf.push_back('.');
      yy_buf.append(data, next_line_begin, line_length);
    }
    if (line_end != data.size()) {
      yy_buf.push_back('\n');
    }
    next_line_begin = line_end + 1;
  };
  auto endline = data.find('\n');
  while (endline != std::string::npos) {
    append_line(endline);
    endline = data.find('\n', next_line_begin);
  }
  append_line(data.size());
  SetScanBuffer(yy_buf, trace_scanning);
}

void AssertionParser::ScanBeginFile(const RE2& goal_comment_regex,
                                    bool trace_scanning) {
  if (file().empty() || file() == "-") {
    Error("will not read goals from stdin");
    exit(EXIT_FAILURE);
  }
  std::string buffer = LoadFileOrDie(file());
  ScanBeginString(goal_comment_regex, buffer, trace_scanning);
}

}  // namespace verifier
}  // namespace kythe
