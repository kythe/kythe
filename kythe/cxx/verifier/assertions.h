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

#ifndef KYTHE_CXX_VERIFIER_ASSERTIONS_H_
#define KYTHE_CXX_VERIFIER_ASSERTIONS_H_

#include <deque>
#include <unordered_map>
#include <vector>

namespace yy {
class AssertionParserImpl;
}

#include "kythe/cxx/verifier/assertion_ast.h"
#include "kythe/cxx/verifier/parser.yy.hh"
#include "re2/re2.h"

namespace kythe {
namespace verifier {

class Verifier;

/// \brief Parses logic programs.
///
/// `AssertionParser` collects together all goals and data that are part of
/// a verification program. This program is then combined with a database of
/// facts (which are merely terms represented in a different, perhaps indexed,
/// format) by the `Verifier`.
class AssertionParser {
 public:
  /// \param trace_lex Dump lexing debug information
  /// \param trace_parse Dump parsing debug information
  explicit AssertionParser(Verifier* verifier, bool trace_lex = false,
                           bool trace_parse = false);

  /// \brief Loads a file containing rules in marked comments.
  /// \param filename The filename of the file to load
  /// \param goal_comment_regex Lines matching this regex are goals. Goals
  /// will be read from the regex's first capture group.
  /// \return true if there were no errors
  bool ParseInlineRuleFile(const std::string& filename, Symbol path,
                           Symbol root, Symbol corpus,
                           const RE2& goal_comment_regex);

  /// \brief Loads a string containing rules in marked comments.
  /// \param content The content to parse and load
  /// \param fake_filename Some string to use when printing errors and locations
  /// \param goal_comment_regex Lines matching this regex are goals. Goals
  /// will be read from the regex's first capture group.
  /// \return true if there were no errors
  bool ParseInlineRuleString(const std::string& content,
                             const std::string& fake_filename, Symbol path,
                             Symbol root, Symbol corpus,
                             const RE2& goal_comment_regex);

  /// \brief The name of the current file being read. It is safe to take
  /// the address of this string (which shares the lifetime of this object.)
  std::string& file() { return files_.back(); }

  /// \brief This `AssertionParser`'s associated `Verifier`.
  Verifier& verifier() { return verifier_; }

  /// \brief All of the goal groups in this `AssertionParser`.
  std::vector<GoalGroup>& groups() { return groups_; }

  /// \brief All of the inspections in this `AssertionParser`.
  std::vector<Inspection>& inspections() { return inspections_; }

  /// \brief Unescapes a string literal (which is expected to include
  /// terminating quotes).
  /// \param yytext literal string to escape
  /// \param out pointer to a string to overwrite with `yytext` unescaped.
  /// \return true if `yytext` was a valid literal string; false otherwise.
  static bool Unescape(const char* yytext, std::string* out);

  /// Should every EVar be added by default to the inspection list?
  void InspectAllEVars() { default_inspect_ = true; }

  /// \brief Check that there are no singleton EVars.
  /// \return true if there were singletons.
  bool CheckForSingletonEVars();

 private:
  friend class yy::AssertionParserImpl;

  /// \brief Sets the scan buffer to a premarked string and turns on
  /// tracing.
  /// \note Implemented in `assertions.lex`.
  void SetScanBuffer(const std::string& scan_buffer, bool trace_scanning);

  /// \brief Resets recorded source text.
  void ResetLine();

  /// \brief Records source text after determining that it does not
  /// begin with a goal comment marker.
  /// \param yytext A 1-length string containing the character to append.
  void AppendToLine(const char* yytext);

  /// \brief Called at the end of an ordinary line of source text to resolve
  /// available forward location references.
  ///
  /// Certain syntactic features (like `@'token`) refer to elements on the
  /// next line of source text. After that next line is buffered using
  /// `AppendToLine`, the lexer calls to `ResolveLocations` to point those
  /// features at the correct locations.
  ///
  /// \return true if all locations could be resolved
  bool ResolveLocations(const yy::location& end_of_line,
                        size_t offset_after_endline, bool end_of_file);

  /// \brief Called by the lexer to save the end location of the current file
  /// or buffer.
  void save_eof(const yy::location& eof, size_t eof_ofs) {
    last_eof_ = eof;
    last_eof_ofs_ = eof_ofs;
  }

  /// \note Implemented by generated code care of flex.
  static int lex(YYSTYPE*, yy::location*, AssertionParser& context);

  /// \brief Used by the lexer and parser to report errors.
  /// \param location Source location where an error occurred.
  /// \param message Text of the error.
  void Error(const yy::location& location, const std::string& message);

  /// \brief Used by the lexer and parser to report errors.
  /// \param message Text of the error.
  void Error(const std::string& message);

  /// \brief Initializes the lexer to scan from file_.
  /// \param goal_comment_regex regex to identify goal comments.
  void ScanBeginFile(const RE2& goal_comment_regex, bool trace_scanning);

  /// \brief Initializes the lexer to scan from a string.
  /// \param goal_comment_regex regex to identify goal comments.
  void ScanBeginString(const RE2& goal_comment_regex, const std::string& data,
                       bool trace_scanning);

  /// \brief Handles end-of-scan actions and destroys any buffers.
  /// \note Implemented in `assertions.lex`.
  void ScanEnd(const yy::location& eof_loc, size_t eof_loc_ofs);
  AstNode** PopNodes(size_t node_count);
  void PushNode(AstNode* node);
  void AppendGoal(size_t group_id, AstNode* goal);

  /// \brief Generates deduplicated `Identifier`s or `EVar`s.
  /// \param location Source location of the token.
  /// \param for_token Token to check.
  /// \return An `EVar` if `for_token` starts with a capital letter;
  /// an `Identifier` otherwise.
  /// \sa CreateEVar, CreateIdentifier
  AstNode* CreateAtom(const yy::location& location,
                      const std::string& for_token);

  /// \brief Generates an equality constraint between the lhs and the rhs.
  /// \param location Source location of the "=" token.
  /// \param lhs The lhs of the equality.
  /// \param rhs The rhs of the equality.
  AstNode* CreateEqualityConstraint(const yy::location& location, AstNode* lhs,
                                    AstNode* rhs);

  /// \brief Generates deduplicated `EVar`s.
  /// \param location Source location of the token.
  /// \param for_token Token to use.
  /// \return A new `EVar` if `for_token` has not yet been made into
  /// an `EVar` already, or the previous `EVar` returned the last
  /// time `CreateEVar` was called.
  EVar* CreateEVar(const yy::location& location, const std::string& for_token);

  /// \brief Generates deduplicated `Identifier`s.
  /// \param location Source location of the text.
  /// \param for_text text to use.
  /// \return A new `Identifier` if `for_text` has not yet been made into
  /// an `Identifier` already, or the previous `Identifier` returned the last
  /// time `CreateIdenfier` was called.
  Identifier* CreateIdentifier(const yy::location& location,
                               const std::string& for_text);

  /// \brief Creates an anonymous `EVar` to implement the `_` token.
  /// \param location Source location of the token.
  AstNode* CreateDontCare(const yy::location& location);

  /// \brief Adds an inspect post-action to the current goal.
  /// \param location Source location for the inspection.
  /// \param for_exp Expression to inspect.
  /// \return An inspection record.
  AstNode* CreateInspect(const yy::location& location,
                         const std::string& inspect_id, AstNode* to_inspect);

  void PushLocationSpec(const std::string& for_token);

  /// \brief Pushes a relative location spec (@token:+2).
  void PushRelativeLocationSpec(const std::string& for_token,
                                const std::string& relative_spec);

  /// \brief Pushes an absolute location spec (@token:1234).
  void PushAbsoluteLocationSpec(const std::string& for_token,
                                const std::string& absolute);

  /// \brief Changes the last-pushed location spec to match the `match_spec`th
  /// instance of its match string.
  void SetTopLocationSpecMatchNumber(const std::string& number);

  AstNode* CreateAnchorSpec(const yy::location& location);

  /// \brief Generates a new offset spec (equivalent to a string literal).
  /// \param location The location in the goal text of this offset spec.
  /// \param at_end should this offset spec be at the end of the search string?
  AstNode* CreateOffsetSpec(const yy::location& location, bool at_end);

  AstNode* CreateSimpleEdgeFact(const yy::location& location, AstNode* edge_lhs,
                                const std::string& literal_kind,
                                AstNode* edge_rhs, AstNode* ordinal);

  AstNode* CreateSimpleNodeFact(const yy::location& location, AstNode* lhs,
                                const std::string& literal_key, AstNode* value);

  Identifier* PathIdentifierFor(const yy::location& location,
                                const std::string& path_fragment,
                                const std::string& default_root);

  /// \brief Enters a new goal group.
  /// \param location The location for diagnostics.
  /// \param negated true if this group is negated.
  /// Only one goal group may be entered at once.
  void EnterGoalGroup(const yy::location& location, bool negated);

  /// \brief Exits the last-entered goal group.
  void ExitGoalGroup(const yy::location& location);

  /// \brief The current goal group.
  size_t group_id() const {
    if (inside_goal_group_) {
      return groups_.size() - 1;
    } else {
      return 0;
    }
  }

  Verifier& verifier_;

  /// The arena from the verifier; needed by the parser implementation.
  Arena* arena_ = nullptr;

  std::vector<GoalGroup> groups_;
  bool inside_goal_group_ = false;
  /// \brief A record for some text to be matched to its location.
  struct UnresolvedLocation {
    enum Kind {
      kAnchor,       ///< An anchor (@tok).
      kOffsetBegin,  ///< The offset at the start of the location (@^tok).
      kOffsetEnd     ///< The offset at the end of the location (@$tok).
    };
    EVar* anchor_evar;        ///< The EVar to be solved.
    std::string anchor_text;  ///< The text to match.
    size_t line_number;       ///< The line to match text on.
    bool use_line_number;     ///< Whether to match with `line_number` or
                              ///< on the next possible non-goal line.
    size_t group_id;  ///< The group that will own the offset goals, if any.
    Kind kind;        ///< The flavor of UnresolvedLocation we are.
    bool must_be_unambiguous;  ///< If true, anchor_text must match only once.
    int match_number;  ///< If !`must_be_unambiguous`, match this instance of
                       ///< `anchor_text`.
  };
  std::vector<UnresolvedLocation> unresolved_locations_;
  std::vector<AstNode*> node_stack_;
  struct LocationSpec {
    std::string spec;
    int line_offset;
    bool is_absolute;
    bool must_be_unambiguous;
    int match_number;
  };
  std::vector<LocationSpec> location_spec_stack_;
  bool ValidateTopLocationSpec(const yy::location& location,
                               size_t* line_number, bool* use_line_number,
                               bool* must_be_unambiguous, int* match_number);
  /// Files we've parsed or are parsing (pushed onto the back).
  /// Note that location records will have internal pointers to these strings.
  std::deque<std::string> files_;
  std::string line_;
  /// Did we encounter errors during lexing or parsing?
  bool had_errors_ = false;
  /// Save the end-of-file location from the lexer.
  yy::location last_eof_;
  size_t last_eof_ofs_ = 0;
  /// Inspections to be performed after the verifier stops.
  std::vector<Inspection> inspections_;
  /// Context mapping symbols to AST nodes.
  std::unordered_map<Symbol, Identifier*> identifier_context_;
  std::unordered_map<Symbol, EVar*> evar_context_;
  std::unordered_map<EVar*, Symbol> singleton_evars_;
  /// Are we dumping lexer trace information?
  bool trace_lex_ = false;
  /// Are we dumping parser trace information?
  bool trace_parse_ = false;
  /// Should we inspect every user-provided EVar?
  bool default_inspect_ = false;
  /// The current file's path.
  Symbol path_;
  /// The current file's root.
  Symbol root_;
  /// The current file's corpus.
  Symbol corpus_;
};
}  // namespace verifier
}  // namespace kythe

#endif  // KYTHE_CXX_VERIFIER_ASSERTIONS_H_
