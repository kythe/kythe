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

#ifndef KYTHE_CXX_INDEXER_CXX_KYTHE_GRAPH_OBSERVER_H_
#define KYTHE_CXX_INDEXER_CXX_KYTHE_GRAPH_OBSERVER_H_

#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "GraphObserver.h"
#include "KytheClaimClient.h"
#include "KytheGraphRecorder.h"
#include "KytheVFS.h"
#include "kythe/proto/storage.pb.h"

namespace kythe {

class KytheGraphRecorder;

/// \brief Provides information about the provenance and claim status of a node.
///
/// Associates NodeIds with the root, path, and corpus VName fields.
/// As an implementation detail, also contains a flag that determines whether
/// the (file * transcript) pair it came from is statically claimed by the
/// GraphObserver.
class KytheClaimToken : public GraphObserver::ClaimToken {
 public:
  std::string StampIdentity(const std::string &identity) const override {
    std::string stamped = identity;
    if (!vname_.corpus().empty()) {
      stamped.append("#");
      stamped.append(vname_.corpus());
    }
    if (!vname_.root().empty()) {
      stamped.append("#");
      stamped.append(vname_.root());
    }
    if (!vname_.path().empty()) {
      stamped.append("#");
      stamped.append(vname_.path());
    }
    return stamped;
  }

  void *GetClass() const override { return &clazz_; }

  static bool classof(const ClaimToken *t) { return t->GetClass() == &clazz_; }

  /// \brief Marks a VName as belonging to this token.
  void DecorateVName(kythe::proto::VName *target) const {
    target->set_corpus(vname_.corpus());
    target->set_root(vname_.root());
    target->set_path(vname_.path());
  }

  /// \brief Sets a VName that controls the corpus, root and path of claimed
  /// objects.
  void set_vname(const kythe::proto::VName &vname) {
    vname_.set_corpus(vname.corpus());
    vname_.set_root(vname.root());
    vname_.set_path(vname.path());
  }

  /// \sa rough_claimed
  void set_rough_claimed(bool value) { rough_claimed_ = value; }

  /// \brief If true, it is reasonable to assume that this token is
  /// claimed by the current analysis. If false, more investigation
  /// may be required.
  bool rough_claimed() const { return rough_claimed_; }

  bool operator==(const ClaimToken &rhs) const override {
    if (this == &rhs) {
      return true;
    }
    if (const auto *kythe_rhs = clang::dyn_cast<KytheClaimToken>(&rhs)) {
      return kythe_rhs->rough_claimed_ == rough_claimed_ &&
             kythe_rhs->vname_.corpus() == vname_.corpus() &&
             kythe_rhs->vname_.path() == vname_.path() &&
             kythe_rhs->vname_.root() == vname_.root();
    }
    return false;
  }

  bool operator!=(const ClaimToken &rhs) const override {
    if (const auto *kythe_rhs = clang::dyn_cast<KytheClaimToken>(&rhs)) {
      return kythe_rhs->rough_claimed_ != rough_claimed_ ||
             kythe_rhs->vname_.corpus() != vname_.corpus() ||
             kythe_rhs->vname_.path() != vname_.path() ||
             kythe_rhs->vname_.root() != vname_.root();
    }
    return true;
  }

 private:
  static void *clazz_;
  /// The prototypical VName to use for claimed objects.
  kythe::proto::VName vname_;
  bool rough_claimed_ = true;
};

/// \brief Records details in the form of Kythe nodes and edges about elements
/// discovered during indexing to the provided `KytheGraphRecorder`.
class KytheGraphObserver : public GraphObserver {
 public:
  KytheGraphObserver(KytheGraphRecorder *recorder, KytheClaimClient *client,
                     const llvm::IntrusiveRefCntPtr<IndexVFS> vfs)
      : recorder_(recorder), client_(client), vfs_(vfs) {
    assert(recorder_ != nullptr);
    assert(client_ != nullptr);
    default_token_.set_rough_claimed(true);
    type_token_.set_rough_claimed(true);
  }

  NodeId getNodeIdForBuiltinType(const llvm::StringRef &spelling) override {
    NodeId id(getDefaultClaimToken());
    id.Identity = spelling.str();
    id.Identity.append("#builtin");
    return id;
  }

  const ClaimToken *getDefaultClaimToken() const override {
    return &default_token_;
  }

  NodeId recordTappNode(const NodeId &TyconId,
                        const std::vector<const NodeId *> &Params) override;

  NodeId nodeIdForTypeAliasNode(const NameId &AliasName,
                                const NodeId &AliasedType) override;

  NodeId recordTypeAliasNode(const NameId &DeclName,
                             const NodeId &DeclNode) override;

  void recordFunctionNode(const NodeId &Node,
                          Completeness FunctionCompleteness) override;

  void recordCallableNode(const NodeId &Node) override;

  void recordAbsVarNode(const NodeId &Node) override;

  void recordAbsNode(const NodeId &Node) override;

  void recordLookupNode(const NodeId &Node,
                        const llvm::StringRef &Name) override;

  void recordParamEdge(const NodeId &ParamOfNode, uint32_t Ordinal,
                       const NodeId &ParamNode) override;

  void recordRecordNode(const NodeId &Node, RecordKind Kind,
                        Completeness RecordCompleteness) override;

  void recordEnumNode(const NodeId &Node, Completeness Compl,
                      EnumKind Kind) override;

  void recordIntegerConstantNode(const NodeId &Node,
                                 const llvm::APSInt &Value) override;

  NodeId nodeIdForNominalTypeNode(const NameId &TypeName) override;

  NodeId recordNominalTypeNode(const NameId &TypeName) override;

  void recordExtendsEdge(const NodeId &InheritingNodeId,
                         const NodeId &InheritedTypeId, bool IsVirtual,
                         clang::AccessSpecifier AS) override;

  void recordDeclUseLocation(const Range &SourceRange, const NodeId &DeclId,
                             GraphObserver::Claimability Cl) override;

  void recordVariableNode(const NameId &DeclName, const NodeId &DeclNode,
                          Completeness VarCompleteness) override;

  void recordUserDefinedNode(const NameId &Name, const NodeId &Id,
                             const llvm::StringRef &NodeKind,
                             Completeness Compl) override;

  void recordDefinitionRange(const Range &SourceRange,
                             const NodeId &DefnId) override;

  void recordCompletionRange(const Range &SourceRange, const NodeId &DefnId,
                             Specificity Spec) override;

  void recordNamedEdge(const NodeId &Node, const NameId &Name) override;

  void recordTypeSpellingLocation(const Range &SourceRange,
                                  const NodeId &TypeId,
                                  Claimability Cl) override;

  void recordChildOfEdge(const NodeId &ChildNodeId,
                         const NodeId &ParentNodeId) override;

  void recordTypeEdge(const NodeId &TermNodeId,
                      const NodeId &TypeNodeId) override;

  void recordSpecEdge(const NodeId &TermNodeId,
                      const NodeId &TypeNodeId) override;

  void recordInstEdge(const NodeId &TermNodeId,
                      const NodeId &TypeNodeId) override;

  void recordCallableAsEdge(const NodeId &ToCallId,
                            const NodeId &CallableAsId) override;

  void recordCallEdge(const Range &SourceRange, const NodeId &CallerId,
                      const NodeId &CalleeId) override;

  void recordMacroNode(const NodeId &MacroNode) override;

  void recordExpandsRange(const Range &SourceRange,
                          const NodeId &MacroId) override;

  void recordIndirectlyExpandsRange(const Range &SourceRange,
                                    const NodeId &MacroId) override;

  void recordUndefinesRange(const Range &SourceRange,
                            const NodeId &MacroId) override;

  void recordIncludesRange(const Range &SourceRange,
                           const clang::FileEntry *File) override;

  void recordBoundQueryRange(const Range &SourceRange,
                             const NodeId &MacroId) override;

  void recordUnboundQueryRange(const Range &SourceRange,
                               const NameId &MacroName) override;

  void pushFile(clang::SourceLocation BlameLocation,
                clang::SourceLocation Location) override;

  void popFile() override;

  /// \brief Configures the claimant that will be used to make claims.
  void set_claimant(const kythe::proto::VName &vname) { claimant_ = vname; }

  bool claimNode(const NodeId &NodeId) override {
    if (const auto *token = clang::dyn_cast<KytheClaimToken>(NodeId.Token)) {
      return token->rough_claimed();
    }
    return true;
  }

  bool claimRange(const GraphObserver::Range &range) override;

  bool claimLocation(clang::SourceLocation Loc) override;

  /// A representation of the state of the preprocessor.
  using PreprocessorContext = std::string;

  /// \brief Adds a fact about preprocessor contexts.
  /// \param path The (Clang-specific) lookup path for the file.
  /// \param context The source context.
  /// \param offset The offset into the source file.
  /// \param dest_context The context we'll transition into.
  void AddContextInformation(const std::string &path,
                             const PreprocessorContext &context,
                             unsigned offset,
                             const PreprocessorContext &dest_context);

  /// \brief Configures the starting context.
  /// \param context Context to use when the main source file is entered.
  void set_starting_context(const PreprocessorContext &context) {
    starting_context_ = context;
  }

  const ClaimToken *getClaimTokenForLocation(
      const clang::SourceLocation L) override;

  const ClaimToken *getClaimTokenForRange(
      const clang::SourceRange &SR) override;

  /// \brief Appends a representation of `Range` to `Ostream`.
  /// \return true if `Range` was valid; false otherwise.
  bool AppendRangeToStream(llvm::raw_ostream &Ostream,
                           const Range &Range) override;

 private:
  void RecordSourceLocation(clang::SourceLocation source_location,
                            PropertyID offset_id);

  /// \brief Called by `AppendRangeToStream` to recur down a `SourceLocation`.
  ///
  /// A `SourceLocation` may have additional structure due to macro expansions.
  /// This function is used to generate a full serialization of this structure.
  void AppendFullLocationToStream(std::vector<clang::FileID> *posted_fileids,
                                  clang::SourceLocation source_location,
                                  llvm::raw_ostream &Ostream);

  /// \brief Append a stable representation of `loc` to `Ostream`, even if
  /// `loc` is in a temporary buffer.
  void AppendFileBufferSliceHashToStream(clang::SourceLocation loc,
                                         llvm::raw_ostream &Ostream);

  kythe::proto::VName VNameFromNodeId(const GraphObserver::NodeId &node_id);
  kythe::proto::VName VNameFromFileEntry(const clang::FileEntry *file_entry);
  kythe::proto::VName ClaimableVNameFromFileID(const clang::FileID &file_id);
  kythe::proto::VName VNameFromRange(const GraphObserver::Range &range);
  kythe::proto::VName RecordName(const GraphObserver::NameId &name_id);
  kythe::proto::VName RecordAnchor(
      const GraphObserver::Range &source_range,
      const GraphObserver::NodeId &primary_anchored_to,
      EdgeKindID anchor_edge_kind, Claimability claimability);
  kythe::proto::VName RecordAnchor(
      const GraphObserver::Range &source_range,
      const kythe::proto::VName &primary_anchored_to,
      EdgeKindID anchor_edge_kind, Claimability claimability);
  /// Records any deferred nodes, clearing any records of their deferral.
  void RecordDeferredNodes();

  struct RangeHash {
    size_t operator()(const GraphObserver::Range &range) const {
      return std::hash<unsigned>()(
                 range.PhysicalRange.getBegin().getRawEncoding()) ^
             (std::hash<unsigned>()(
                  range.PhysicalRange.getEnd().getRawEncoding())
              << 1) ^
             (std::hash<std::string>()(range.Context.Identity)) ^
             (range.Kind == Range::RangeKind::Wraith ? 1 : 0);
    }
  };
  /// A file we have entered but not left.
  struct FileState {
    PreprocessorContext context;     ///< The context for this file.
    kythe::proto::VName vname;       ///< The file's VName.
    kythe::proto::VName base_vname;  ///< The file's VName without context.
    llvm::sys::fs::UniqueID uid;     ///< The ID Clang uses for this file.
    bool claimed;                    ///< Whether we have claimed this file.
  };
  /// The files we have entered but not left.
  std::vector<FileState> file_stack_;
  /// Files we have previously inspected for claiming. When they refer to
  /// FileEntries, a FileID represents a specific file being included from a
  /// given include position. There will therefore be many FileIDs that map to
  /// one context + header pair; then, many context + header pairs may
  /// map to a single file's VName.
  std::map<clang::FileID, KytheClaimToken> claim_checked_files_;
  /// The `KytheGraphRecorder` used to record graph data. Must not be null.
  KytheGraphRecorder *recorder_;
  /// A VName representing this `GraphObserver`'s claiming authority.
  kythe::proto::VName claimant_;
  /// The starting preprocessor context.
  PreprocessorContext starting_context_;
  /// Maps from #include locations to the resulting preprocessor context.
  using IncludeToContext = std::map<unsigned, PreprocessorContext>;
  /// Maps from preprocessor contexts to context-specific records.
  using ContextToIncludes = std::map<PreprocessorContext, IncludeToContext>;
  /// Maps from file UID to its various context incarnations.
  std::map<llvm::sys::fs::UniqueID, ContextToIncludes> path_to_context_data_;
  /// The `KytheClaimClient` used to reduce output redundancy. Not null.
  KytheClaimClient *client_;
  /// Contains the `FileEntry`s for files we have already recorded.
  /// These pointers are not owned by the `KytheGraphObserver`.
  std::unordered_set<const clang::FileEntry *> recorded_files_;
  /// The set of anchor nodes with locations that have not yet been recorded.
  /// This allows the `GraphObserver` to limit the amount of redundant range
  /// information it emits should an anchor be the source of multiple edges.
  std::unordered_set<Range, RangeHash> deferred_anchors_;
  /// Favor extra memory use during indexing over storing potentially redundant
  /// facts for certain frequently-used node kinds. Since these node kinds
  /// are defined to have structure equivalent to their names (modulo
  /// non-primary types in case of aliases, which may still be stored
  /// redundantly), this will not obscure conflicting-fact errors.
  /// The set of NameIds we have already emitted (identified by
  /// NameId::ToString()).
  std::unordered_set<std::string> written_name_ids_;
  /// The set of type nodes we've emitted so far (identified by
  /// `NodeId::ToString()`).
  std::unordered_set<std::string> written_types_;
  /// The virtual filesystem in use.
  llvm::IntrusiveRefCntPtr<IndexVFS> vfs_;
  /// A neutral claim token.
  KytheClaimToken default_token_;
  /// The claim token to use for structural types.
  KytheClaimToken type_token_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_KYTHE_GRAPH_OBSERVER_H_
