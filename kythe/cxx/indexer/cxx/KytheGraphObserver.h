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

#include "glog/logging.h"

#include "GraphObserver.h"
#include "kythe/cxx/common/indexing/KytheClaimClient.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/indexing/KytheVFS.h"
#include "kythe/cxx/common/kythe_metadata_file.h"
#include "kythe/cxx/indexer/cxx/IndexerASTHooks.h"
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

  /// \brief Marks a VNameRef as belonging to this token.
  /// This token must outlive the VNameRef.
  void DecorateVName(VNameRef *target) const {
    target->corpus =
        llvm::StringRef(vname_.corpus().data(), vname_.corpus().size());
    target->root = llvm::StringRef(vname_.root().data(), vname_.root().size());
    target->path = llvm::StringRef(vname_.path().data(), vname_.path().size());
  }

  /// \brief Sets a VName that controls the corpus, root and path of claimed
  /// objects.
  void set_vname(const kythe::proto::VName &vname) {
    vname_.set_corpus(vname.corpus());
    vname_.set_root(vname.root());
    vname_.set_path(vname.path());
  }

  const kythe::proto::VName &vname() const { return vname_; }

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
                     const MetadataSupports *meta_supports,
                     const llvm::IntrusiveRefCntPtr<IndexVFS> vfs,
                     ProfilingCallback ReportProfileEventCallback)
      : recorder_(CHECK_NOTNULL(recorder)),
        client_(CHECK_NOTNULL(client)),
        meta_supports_(CHECK_NOTNULL(meta_supports)),
        vfs_(vfs) {
    default_token_.set_rough_claimed(true);
    type_token_.set_rough_claimed(true);
    ReportProfileEvent = ReportProfileEventCallback;
    RegisterBuiltins();
    EmitMetaNodes();
  }

  NodeId getNodeIdForBuiltinType(const llvm::StringRef &spelling) override {
    const auto &info = builtins_.find(spelling.str());
    if (info == builtins_.end()) {
      LOG(ERROR) << "Missing builtin " << spelling.str();
      builtins_.emplace(
          spelling.str(),
          Builtin{NodeId::CreateUncompressed(getDefaultClaimToken(),
                                             spelling.str() + "#builtin"),
                  EscapeForFormatLiteral(spelling), true});
      auto *new_builtin = &builtins_.find(spelling.str())->second;
      EmitBuiltin(new_builtin);
      return new_builtin->node_id;
    }
    if (!info->second.emitted) {
      EmitBuiltin(&info->second);
    }
    return info->second.node_id;
  }

  const KytheClaimToken *getDefaultClaimToken() const override {
    return &default_token_;
  }

  void applyMetadataFile(clang::FileID ID, const clang::FileEntry *FE) override;
  void StopDeferringNodes() { deferring_nodes_ = false; }
  void DropRedundantWraiths() { drop_redundant_wraiths_ = true; }
  void Delimit() override { recorder_->PushEntryGroup(); }
  void Undelimit() override { recorder_->PopEntryGroup(); }

  NodeId recordTappNode(const NodeId &TyconId,
                        const std::vector<const NodeId *> &Params) override;

  NodeId recordTsigmaNode(const std::vector<const NodeId *> &Params) override;

  NodeId nodeIdForTypeAliasNode(const NameId &AliasName,
                                const NodeId &AliasedType) override;

  NodeId recordTypeAliasNode(const NameId &DeclName, const NodeId &DeclNode,
                             const std::string &Format) override;

  void recordFunctionNode(const NodeId &Node, Completeness FunctionCompleteness,
                          FunctionSubkind Subkind,
                          const std::string &Format) override;

  void recordAbsVarNode(const NodeId &Node) override;

  void recordAbsNode(const NodeId &Node) override;

  void recordLookupNode(const NodeId &Node,
                        const llvm::StringRef &Name) override;

  void recordParamEdge(const NodeId &ParamOfNode, uint32_t Ordinal,
                       const NodeId &ParamNode) override;

  void recordRecordNode(const NodeId &Node, RecordKind Kind,
                        Completeness RecordCompleteness,
                        const std::string &Format) override;

  void recordEnumNode(const NodeId &Node, Completeness Compl,
                      EnumKind Kind) override;

  void recordIntegerConstantNode(const NodeId &Node,
                                 const llvm::APSInt &Value) override;

  NodeId nodeIdForNominalTypeNode(const NameId &TypeName) override;

  NodeId recordNominalTypeNode(const NameId &TypeName,
                               const std::string &Format,
                               const NodeId *Parent) override;

  void recordExtendsEdge(const NodeId &InheritingNodeId,
                         const NodeId &InheritedTypeId, bool IsVirtual,
                         clang::AccessSpecifier AS) override;

  void recordDeclUseLocation(const Range &SourceRange, const NodeId &DeclId,
                             GraphObserver::Claimability Cl) override;

  void recordVariableNode(const NameId &DeclName, const NodeId &DeclNode,
                          Completeness VarCompleteness, VariableSubkind Subkind,
                          const std::string &Format) override;

  void recordNamespaceNode(const NameId &DeclName, const NodeId &DeclNode,
                           const std::string &Format) override;

  void recordUserDefinedNode(const NameId &Name, const NodeId &Id,
                             const llvm::StringRef &NodeKind,
                             Completeness Compl) override;

  void recordFullDefinitionRange(const Range &SourceRange,
                                 const NodeId &DefnId) override;

  void recordDefinitionBindingRange(const Range &BindingRange,
                                    const NodeId &DefnId) override;

  void recordDefinitionRangeWithBinding(const Range &SourceRange,
                                        const Range &BindingRange,
                                        const NodeId &DefnId) override;

  void recordDocumentationRange(const Range &SourceRange,
                                const NodeId &DocId) override;

  void recordDocumentationText(const NodeId &DocId, const std::string &DocText,
                               const std::vector<NodeId> &DocLinks) override;

  void recordDeclUseLocationInDocumentation(const Range &SourceRange,
                                            const NodeId &DeclId) override;

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

  void recordSpecEdge(const NodeId &TermNodeId, const NodeId &TypeNodeId,
                      Confidence Conf) override;

  void recordInstEdge(const NodeId &TermNodeId, const NodeId &TypeNodeId,
                      Confidence Conf) override;

  void recordOverridesEdge(const NodeId &Overrider,
                           const NodeId &BaseObject) override;

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

  bool isMainSourceFileRelatedLocation(clang::SourceLocation Location) override;

  void AppendMainSourceFileIdentifierToStream(
      llvm::raw_ostream &Ostream) override;

  /// \brief Configures the claimant that will be used to make claims.
  void set_claimant(const kythe::proto::VName &vname) { claimant_ = vname; }

  bool claimNode(const NodeId &NodeId) override {
    if (const auto *token =
            clang::dyn_cast<KytheClaimToken>(NodeId.getToken())) {
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

  KytheClaimToken *getClaimTokenForLocation(
      const clang::SourceLocation L) override;

  KytheClaimToken *getClaimTokenForRange(const clang::SourceRange &SR) override;

  KytheClaimToken *getNamespaceClaimToken(clang::SourceLocation Loc) override;

  KytheClaimToken *getAnonymousNamespaceClaimToken(
      clang::SourceLocation Loc) override;

  /// \brief Appends a representation of `Range` to `Ostream`.
  void AppendRangeToStream(llvm::raw_ostream &Ostream,
                           const Range &Range) override;

  /// \brief Set whether we're willing to drop data to avoid redundancy.
  ///
  /// The resulting output will be missing nodes and edges depending on
  /// various factors, including whether claim transcripts are available in the
  /// input translation units and whether template instantations are being
  /// indexed.
  void set_lossy_claiming(bool value) { lossy_claiming_ = value; }

  bool lossy_claiming() const override { return lossy_claiming_; }

 private:
  void RecordSourceLocation(const VNameRef &vname,
                            clang::SourceLocation source_location,
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

  VNameRef VNameRefFromNodeId(const GraphObserver::NodeId &node_id);
  kythe::proto::VName VNameFromFileEntry(const clang::FileEntry *file_entry);
  kythe::proto::VName ClaimableVNameFromFileID(const clang::FileID &file_id);
  kythe::proto::VName VNameFromRange(const GraphObserver::Range &range);
  MaybeFew<kythe::proto::VName> RecordName(
      const GraphObserver::NameId &name_id);
  void RecordAnchor(const GraphObserver::Range &source_range,
                    const GraphObserver::NodeId &primary_anchored_to,
                    EdgeKindID anchor_edge_kind, Claimability claimability);
  void RecordAnchor(const GraphObserver::Range &source_range,
                    const kythe::proto::VName &primary_anchored_to,
                    EdgeKindID anchor_edge_kind, Claimability claimability);
  /// Records a Range.
  void RecordRange(const proto::VName &range_vname,
                   const GraphObserver::Range &range);
  /// Execute metadata actions for `defines` edges.
  void MetaHookDefines(const MetadataFile &meta, const VNameRef &anchor,
                       unsigned range_begin, unsigned range_end,
                       const VNameRef &def);

  struct RangeHash {
    size_t operator()(const GraphObserver::Range &range) const {
      return std::hash<unsigned>()(
                 range.PhysicalRange.getBegin().getRawEncoding()) ^
             (std::hash<unsigned>()(
                  range.PhysicalRange.getEnd().getRawEncoding())
              << 1) ^
             (std::hash<std::string>()(range.Context.getRawIdentity())) ^
             (range.Kind == Range::RangeKind::Physical
                  ? 0
                  : (range.Kind == Range::RangeKind::Wraith ? 1 : 2));
    }
  };

  struct RangeEdge {
    clang::SourceRange PhysicalRange;
    EdgeKindID EdgeKind;
    GraphObserver::NodeId EdgeTarget;
    size_t Hash;
    bool operator==(const RangeEdge &range) const {
      return std::tie(Hash, PhysicalRange, EdgeKind, EdgeTarget) ==
             std::tie(range.Hash, range.PhysicalRange, range.EdgeKind,
                      range.EdgeTarget);
    }
    static size_t ComputeHash(const clang::SourceRange &PhysicalRange,
                              EdgeKindID EdgeKind,
                              const GraphObserver::NodeId EdgeTarget) {
      return std::hash<unsigned>()(PhysicalRange.getBegin().getRawEncoding()) ^
             (std::hash<unsigned>()(PhysicalRange.getEnd().getRawEncoding())
              << 1) ^
             (std::hash<unsigned>()(static_cast<unsigned>(EdgeKind))) ^
             (std::hash<std::string>()(EdgeTarget.getRawIdentity()));
    }
  };

  struct ContextFreeRangeEdgeHash {
    size_t operator()(const RangeEdge &range) const { return range.Hash; }
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
  /// A map from FileIDs to associated metadata.
  std::multimap<clang::FileID, std::shared_ptr<MetadataFile>> meta_;
  /// All files that were ever reached through a header file, including header
  /// files themselves.
  std::set<llvm::sys::fs::UniqueID> transitively_reached_through_header_;
  /// A location in the main source file.
  clang::SourceLocation main_source_file_loc_;
  /// A claim token in the main source file.
  KytheClaimToken *main_source_file_token_ = nullptr;
  /// Files we have previously inspected for claiming. When they refer to
  /// FileEntries, a FileID represents a specific file being included from a
  /// given include position. There will therefore be many FileIDs that map to
  /// one context + header pair; then, many context + header pairs may
  /// map to a single file's VName.
  std::map<clang::FileID, KytheClaimToken> claim_checked_files_;
  /// Maps from claim tokens to claim tokens with path and root dropped.
  std::map<KytheClaimToken *, KytheClaimToken> namespace_tokens_;
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
  /// A set of (source range, edge kind, target node) tuples, used if
  /// drop_redundant_wraiths_ is asserted.
  std::unordered_set<RangeEdge, ContextFreeRangeEdgeHash> range_edges_;
  /// If true, anchors and edges from a given physical source location will
  /// be dropped if they were previously emitted from the same location
  /// with the same edge kind to the same target.
  bool drop_redundant_wraiths_ = false;
  /// Favor extra memory use during indexing over storing potentially redundant
  /// facts for certain frequently-used node kinds. Since these node kinds
  /// are defined to have structure equivalent to their names (modulo
  /// non-primary types in case of aliases, which may still be stored
  /// redundantly), this will not obscure conflicting-fact errors.
  /// The set of NameIds we have already emitted (identified by
  /// NameId::ToString()).
  std::unordered_set<std::string> written_name_ids_;
  /// The set of doc nodes we've emitted so far (identified by
  /// `NodeId::ToString()`).
  std::unordered_set<std::string> written_docs_;
  /// The set of type nodes we've emitted so far (identified by
  /// `NodeId::ToString()`).
  std::unordered_set<std::string> written_types_;
  /// The set of namespace nodes we've emitted so far (identified by
  /// `NodeId::ToString()`).
  std::unordered_set<std::string> written_namespaces_;
  /// Whether to try and locally deduplicate nodes.
  bool deferring_nodes_ = true;
  /// \brief Enabled metadata import support.
  const MetadataSupports *const meta_supports_;
  /// The virtual filesystem in use.
  llvm::IntrusiveRefCntPtr<IndexVFS> vfs_;
  /// A neutral claim token.
  KytheClaimToken default_token_;
  /// The claim token to use for structural types.
  KytheClaimToken type_token_;
  /// Possibly drop data for the greater good of eliminating redundancy.
  bool lossy_claiming_ = false;
  /// Information about builtin nodes.
  struct Builtin {
    /// This Builtin's NodeId.
    NodeId node_id;
    /// A format string for this Builtin.
    std::string format;
    /// Whether this Builtin has been emitted.
    bool emitted;
  };
  /// Add known Builtins to builtins_.
  void RegisterBuiltins();
  /// Emit a particular Builtin.
  void EmitBuiltin(Builtin *builtin);
  /// Emit entries for C++ meta nodes.
  void EmitMetaNodes();
  /// Registered builtins.
  std::map<std::string, Builtin> builtins_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_KYTHE_GRAPH_OBSERVER_H_
