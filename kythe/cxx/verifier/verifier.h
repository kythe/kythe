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

#ifndef KYTHE_CXX_VERIFIER_H_
#define KYTHE_CXX_VERIFIER_H_

#include <functional>
#include <optional>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/types/span.h"
#include "assertions.h"
#include "kythe/proto/common.pb.h"
#include "kythe/proto/storage.pb.h"

namespace kythe {
namespace verifier {
/// \brief Runs logic programs.
///
/// The `Verifier` combines an `AssertionContext` with a database of Kythe
/// facts. It can be used to determine whether the goals specified in the
/// `AssertionContext` are satisfiable.
class Verifier {
 public:
  /// \param trace_lex Dump lexing debug information
  /// \param trace_parse Dump parsing debug information
  explicit Verifier(bool trace_lex = false, bool trace_parse = false);

  /// \brief Loads an in-memory source file.
  /// \param filename The name to use for the file; may be blank.
  /// \param vname The AstNode of the vname for the file.
  /// \param text The symbol for the text to load
  /// \return false if we failed.
  bool LoadInMemoryRuleFile(const std::string& filename, AstNode* vname,
                            Symbol text);

  /// \brief Loads a source file with goal comments indicating rules and data.
  /// The VName for the source file will be determined by matching its content
  /// against file nodes.
  /// \param filename The filename to load
  /// \return false if we failed
  bool LoadInlineRuleFile(const std::string& filename);

  /// \brief Loads a text proto with goal comments indicating rules and data.
  /// The VName for the source file will be blank.
  /// \param file_data The data to load
  /// \param path the path to use for anchors
  /// \param root the root to use for anchors
  /// \param corpus the corpus to use for anchors
  /// \return false if we failed
  bool LoadInlineProtoFile(const std::string& file_data,
                           absl::string_view path = "",
                           absl::string_view root = "",
                           absl::string_view corpus = "");

  /// \brief During verification, ignore duplicate facts.
  void IgnoreDuplicateFacts();

  /// \brief During verification, ignore conflicting /kythe/code facts.
  void IgnoreCodeConflicts();

  /// \brief Save results of verification keyed by inspection label.
  void SaveEVarAssignments();

  /// \brief Dump all goals to standard out.
  void ShowGoals();

  /// \brief Prints out a particular goal with its original source location
  /// to standard error.
  /// \param group_index The index of the goal's group.
  /// \param goal_index The index of the goal to print
  /// \sa highest_goal_reached, highest_group_reached
  void DumpErrorGoal(size_t group_index, size_t goal_index);

  /// \brief Dump known facts to standard out as a GraphViz graph.
  void DumpAsDot();

  /// \brief Dump known facts to standard out as JSON.
  void DumpAsJson();

  /// \brief Attempts to satisfy all goals from all loaded rule files and facts.
  /// \param inspect function to call on any inspection request
  /// \return true if all goals could be satisfied.
  bool VerifyAllGoals(std::function<bool(Verifier* context, const Inspection&,
                                         std::string_view)>
                          inspect);

  /// \brief Attempts to satisfy all goals from all loaded rule files and facts.
  /// \param inspect function to call on any inspection request
  /// \return true if all goals could be satisfied.
  bool VerifyAllGoals(
      std::function<bool(Verifier* context, const Inspection&)> inspect) {
    return VerifyAllGoals([&](Verifier* v, const Inspection& i,
                              std::string_view) { return inspect(v, i); });
  }

  /// \brief Attempts to satisfy all goals from all loaded rule files and facts.
  /// \return true if all goals could be satisfied.
  bool VerifyAllGoals();

  /// \brief Adds a single Kythe fact to the database.
  /// \param database_name some name used to define the database; should live
  /// as long as the `Verifier`. Used only for diagnostics.
  /// \param fact_id some identifier for the fact. Used only for diagnostics.
  /// \return false if something went wrong.
  bool AssertSingleFact(std::string* database_name, unsigned int fact_id,
                        const kythe::proto::Entry& entry);

  /// \brief Perform basic well-formedness checks on the input database.
  /// \pre The database contains only fact-shaped terms, as generated by
  /// `AssertSingleFact`.
  /// \return false if the database was not well-formed.
  bool PrepareDatabase();

  /// Arena for allocating memory for both static data loaded from the database
  /// and dynamic data allocated during the course of evaluation.
  Arena* arena() { return &arena_; }

  /// Symbol table for uniquing strings.
  SymbolTable* symbol_table() { return &symbol_table_; }

  /// \brief Allocates an identifier for some token.
  /// \param location The source location for the identifier.
  /// \param token The text of the identifier.
  /// \return An `Identifier`. This may not be unique.
  Identifier* IdentifierFor(const yy::location& location,
                            const std::string& token);

  /// \brief Stringifies an integer, then makes an identifier out of it.
  /// \param location The source location for the identifier.
  /// \param integer The integer to stringify.
  /// \return An `Identifier`. This may not be unique.
  Identifier* IdentifierFor(const yy::location& location, int integer);

  /// \brief Convenience function to make `(App head (Tuple values))`.
  /// \param location The source location for the predicate.
  /// \param head The lhs of the `App` to allocate.
  /// \param values The body of the `Tuple` to allocate.
  AstNode* MakePredicate(const yy::location& location, AstNode* head,
                         absl::Span<AstNode* const> values);

  /// \brief The head used for equality predicates.
  Identifier* eq_id() { return eq_id_; }

  /// \brief The head used for any VName predicate.
  AstNode* vname_id() { return vname_id_; }

  /// \brief The head used for any Fact predicate.
  AstNode* fact_id() { return fact_id_; }

  /// \brief The fact kind for an root/empty fact label.
  AstNode* root_id() { return root_id_; }

  /// \brief The empty string as an identifier.
  AstNode* empty_string_id() { return empty_string_id_; }

  /// \brief The fact kind for an edge ordinal.
  AstNode* ordinal_id() { return ordinal_id_; }

  /// \brief The fact kind used to assign a node its kind (eg /kythe/node/kind).
  AstNode* kind_id() { return kind_id_; }

  /// \brief The fact kind used for an anchor.
  AstNode* anchor_id() { return anchor_id_; }

  /// \brief The fact kind used for a file.
  AstNode* file_id() { return file_id_; }

  /// \brief Object for parsing and storing assertions.
  AssertionParser* parser() { return &parser_; }

  /// \brief Returns the highest group index the verifier reached during
  /// solving.
  size_t highest_group_reached() const { return highest_group_reached_; }

  /// \brief Returns the highest goal index the verifier reached during
  /// solving.
  size_t highest_goal_reached() const { return highest_goal_reached_; }

  /// \brief Change the regex used to identify goals in source text.
  /// \return false on failure.
  bool SetGoalCommentRegex(const std::string& regex, std::string* error);

  /// \brief Use a prefix to match goals in source text.
  void SetGoalCommentPrefix(const std::string& it);

  /// \brief Look for assertions in file node text.
  void UseFileNodes() { assertions_from_file_nodes_ = true; }

  /// \brief Convert MarkedSource-valued facts to graphs.
  void ConvertMarkedSource() { convert_marked_source_ = true; }

  /// \brief Show anchor locations in graph dumps (instead of @).
  void ShowAnchors() { show_anchors_ = true; }

  /// \brief Show VNames for nodes which also have labels in graph dumps.
  void ShowLabeledVnames() { show_labeled_vnames_ = true; }

  /// \brief Show the /kythe and /kythe/edge prefixes in graph dumps.
  void ShowFactPrefix() { show_fact_prefix_ = true; }

  /// \brief Elide unlabeled nodes from graph dumps.
  void ElideUnlabeled() { show_unlabeled_ = false; }

  /// \brief Check for singleton EVars.
  /// \return true if there were singletons.
  bool CheckForSingletonEVars() { return parser_.CheckForSingletonEVars(); }

  /// \brief Don't search for file vnames.
  void IgnoreFileVnames() { file_vnames_ = false; }

  /// \brief Use the fast solver.
  void UseFastSolver(bool value) { use_fast_solver_ = value; }

  /// \brief Gets a string representation of `i`.
  /// \deprecated Inspection callbacks will be provided with strings and
  /// will no longer have access to the internal AST.
  std::string InspectionString(const Inspection& i);

 private:
  using InternedVName = std::tuple<Symbol, Symbol, Symbol, Symbol, Symbol>;

  /// \brief Interns an AST node known to be a VName.
  /// \param node the node to intern.
  InternedVName InternVName(AstNode* node);

  /// \brief Generate a VName that will not conflict with any other VName.
  AstNode* NewUniqueVName(const yy::location& loc);

  /// \brief Converts an encoded /kythe/code fact to a form that's useful
  /// to the verifier.
  /// \param loc The location to use in diagnostics.
  /// \return null if something went wrong; otherwise, an AstNode corresponding
  /// to a VName of a synthetic node for `code_data`.
  AstNode* ConvertCodeFact(const yy::location& loc,
                           const std::string& code_data);

  /// \brief Converts an encoded /kythe/code/json fact to a form that's useful
  /// to the verifier.
  /// \param loc The location to use in diagnostics.
  /// \return null if something went wrong; otherwise, an AstNode corresponding
  /// to a VName of a synthetic node for `code_data`.
  AstNode* ConvertCodeJsonFact(const yy::location& loc,
                               const std::string& code_data);

  /// \brief Converts a MarkedSource message to a form that's useful
  /// to the verifier.
  /// \param loc The location to use in diagnostics.
  /// \return null if something went wrong; otherwise, an AstNode corresponding
  /// to a VName of a synthetic node for `marked_source`.
  AstNode* ConvertMarkedSource(
      const yy::location& loc,
      const kythe::proto::common::MarkedSource& marked_source);

  /// \brief Converts a VName proto to its AST representation.
  AstNode* ConvertVName(const yy::location& location,
                        const kythe::proto::VName& vname);

  /// \brief Adds an anchor VName.
  void AddAnchor(AstNode* vname, size_t begin, size_t end) {
    anchors_.emplace(std::make_pair(begin, end), vname);
  }

  /// \brief Processes a fact tuple for the fast solver.
  /// \param tuple the five-tuple representation of a fact
  /// \return true if successful.
  bool ProcessFactTupleForFastSolver(Tuple* tuple);

  /// \sa parser()
  AssertionParser parser_;

  /// \sa arena()
  Arena arena_;

  /// \sa symbol_table()
  SymbolTable symbol_table_;

  /// All known facts.
  Database facts_;

  /// Maps anchor offsets to anchor VName tuples.
  AnchorMap anchors_;

  /// Has the database been prepared?
  bool database_prepared_ = false;

  /// Ignore duplicate facts during verification?
  bool ignore_dups_ = false;

  /// Ignore conflicting /kythe/code facts during verification?
  bool ignore_code_conflicts_ = false;

  /// Filename to use for builtin constants.
  std::string builtin_location_name_;

  /// Location to use for builtin constants.
  yy::location builtin_location_;

  /// Node to use for the `=` identifier.
  Identifier* eq_id_;

  /// Node to use for the `vname` constant.
  AstNode* vname_id_;

  /// Node to use for the `fact` constant.
  AstNode* fact_id_;

  /// Node to use for the `/` constant.
  AstNode* root_id_;

  /// Node to use for the empty string constant.
  AstNode* empty_string_id_;

  /// Node to use for the `/kythe/ordinal` constant.
  AstNode* ordinal_id_;

  /// Node to use for the `/kythe/node/kind` constant.
  AstNode* kind_id_;

  /// Node to use for the `anchor` constant.
  AstNode* anchor_id_;

  /// Node to use for the `/kythe/loc/start` constant.
  AstNode* start_id_;

  /// Node to use for the `/kythe/loc/end` constant.
  AstNode* end_id_;

  /// Node to use for the `file` node kind.
  AstNode* file_id_;

  /// Node to use for the `text` fact kind.
  AstNode* text_id_;

  /// Node to use for the `code` fact kind. The fact value should be a
  /// serialized kythe.proto.MarkedSource message.
  AstNode* code_id_;

  /// Node to use for the `code/json` fact kind. The fact value should be a
  /// JSON-serialized kythe.proto.MarkedSource message.
  AstNode* code_json_id_;

  /// The highest goal group reached during solving (often the culprit for why
  /// the solution failed).
  size_t highest_group_reached_ = 0;

  /// The highest goal reached during solving (often the culprit for why
  /// the solution failed).
  size_t highest_goal_reached_ = 0;

  /// Whether we save assignments to EVars (by inspection label).
  bool saving_assignments_ = false;

  /// A map from inspection label to saved assignment. Note that
  /// duplicate labels will overwrite one another. This means that
  /// it's important to disambiguate cases where this is likely
  /// (e.g., we add line and column information to labels we generate
  /// for anchors).
  std::map<std::string, AstNode*> saved_assignments_;

  /// Maps from pretty-printed vnames to (parsed) file node text.
  std::map<std::string, Symbol> fake_files_;

  /// Read assertions from file nodes.
  bool assertions_from_file_nodes_ = false;

  /// The regex to look for to identify goal comments. Should have one match
  /// group.
  std::unique_ptr<RE2> goal_comment_regex_;

  /// If true, convert MarkedSource-valued facts to subgraphs. If false,
  /// MarkedSource-valued facts will be replaced with opaque but unique
  /// identifiers.
  bool convert_marked_source_ = false;

  /// If true, show anchor locations in graph dumps (instead of @).
  bool show_anchors_ = false;

  /// If true, show unlabeled nodes in graph dumps.
  bool show_unlabeled_ = true;

  /// If true, show VNames for labeled nodes in graph dumps.
  bool show_labeled_vnames_ = false;

  /// If true, include the /kythe and /kythe/edge prefix on facts and edges.
  bool show_fact_prefix_ = false;

  /// Identifier for MarkedSource child edges.
  AstNode* marked_source_child_id_;

  /// Identifier for MarkedSource code edges.
  AstNode* marked_source_code_edge_id_;

  /// Identifier for MarkedSource BOX kinds.
  AstNode* marked_source_box_id_;

  /// Identifier for MarkedSource TYPE kinds.
  AstNode* marked_source_type_id_;

  /// Identifier for MarkedSource PARAMETER kinds.
  AstNode* marked_source_parameter_id_;

  /// Identifier for MarkedSource IDENTIFIER kinds.
  AstNode* marked_source_identifier_id_;

  /// Identifier for MarkedSource CONTEXT kinds.
  AstNode* marked_source_context_id_;

  /// Identifier for MarkedSource INITIALIZER kinds.
  AstNode* marked_source_initializer_id_;

  /// Identifier for MarkedSource MODIFIER kinds.
  AstNode* marked_source_modifier_id_;

  /// Identifier for MarkedSource PARAMETER_LOOKUP_BY_PARAM kinds.
  AstNode* marked_source_parameter_lookup_by_param_id_;

  /// Identifier for MarkedSource LOOKUP_BY_PARAM kinds.
  AstNode* marked_source_lookup_by_param_id_;

  /// Identifier for MarkedSource PARAMETER_LOOKUP_BY_TPARAM kinds.
  AstNode* marked_source_parameter_lookup_by_tparam_id_;

  /// Identifier for MarkedSource LOOKUP_BY_TPARAM kinds.
  AstNode* marked_source_lookup_by_tparam_id_;

  /// Identifier for MarkedSource LOOKUP_BY_PARAM_WITH_DEFAULTS kinds.
  AstNode* marked_source_parameter_lookup_by_param_with_defaults_id_;

  /// Identifier for MarkedSource LOOKUP_BY_TYPED kinds.
  AstNode* marked_source_lookup_by_typed_id_;

  /// Identifier for MarkedSource kind facts.
  AstNode* marked_source_kind_id_;

  /// Identifier for MarkedSource pre_text facts.
  AstNode* marked_source_pre_text_id_;

  /// Identifier for MarkedSource post_child_text facts.
  AstNode* marked_source_post_child_text_id_;

  /// Identifier for MarkedSource post_text facts.
  AstNode* marked_source_post_text_id_;

  /// Identifier for MarkedSource lookup_index facts.
  AstNode* marked_source_lookup_index_id_;

  /// Identifier for MarkedSource default_children_count facts.
  AstNode* marked_source_default_children_count_id_;

  /// Identifier for MarkedSource add_final_list_token facts.
  AstNode* marked_source_add_final_list_token_id_;

  /// Identifier for MarkedSource link edges.
  AstNode* marked_source_link_id_;

  /// Identifier for MarkedSource true values.
  AstNode* marked_source_true_id_;

  /// Identifier for MarkedSource false values.
  AstNode* marked_source_false_id_;

  /// Maps from file content to (verified) VName.
  absl::flat_hash_map<Symbol, AstNode*> content_to_vname_;

  /// Find file vnames by examining file content.
  bool file_vnames_ = true;

  /// Use the fast solver.
  bool use_fast_solver_ = false;

  /// Sentinel value for a known file.
  Symbol known_file_sym_;

  /// Sentinel value for a known nonfile.
  Symbol known_not_file_sym_;

  /// Maps VNames to known_file_sym_, known_not_file_sym_, or file text.
  absl::flat_hash_map<InternedVName, Symbol> fast_solver_files_;
};

}  // namespace verifier
}  // namespace kythe

#endif  // KYTHE_CXX_VERIFIER_H_
