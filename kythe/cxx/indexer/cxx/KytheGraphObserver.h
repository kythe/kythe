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

#ifndef KYTHE_CXX_INDEXER_CXX_KYTHE_GRAPH_OBSERVER_H_
#define KYTHE_CXX_INDEXER_CXX_KYTHE_GRAPH_OBSERVER_H_

#include <functional>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "GraphObserver.h"
#include "absl/container/flat_hash_set.h"
#include "absl/log/check.h"
#include "absl/log/die_if_null.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/kythe_metadata_file.h"
#include "kythe/cxx/extractor/language.h"
#include "kythe/cxx/indexer/cxx/IndexerASTHooks.h"
#include "kythe/cxx/indexer/cxx/KytheClaimClient.h"
#include "kythe/cxx/indexer/cxx/KytheVFS.h"
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
  std::string StampIdentity(const std::string& identity) const override {
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

  uintptr_t GetClass() const final { return kTokenClass; }

  static bool classof(const ClaimToken* t) {
    return t->GetClass() == kTokenClass;
  }

  /// \brief Marks a VNameRef as belonging to this token.
  /// This token must outlive the VNameRef.
  void DecorateVName(VNameRef* target) const {
    target->set_corpus(vname_.corpus());
    target->set_root(vname_.root());
    target->set_path(vname_.path());
  }

  /// \brief Sets a VName that controls the corpus, root and path of claimed
  /// objects.
  void set_vname(const kythe::proto::VName& vname) {
    vname_.set_corpus(vname.corpus());
    vname_.set_root(vname.root());
    vname_.set_path(vname.path());
  }

  const kythe::proto::VName& vname() const { return vname_; }
  kythe::proto::VName* mutable_vname() { return &vname_; }

  /// \sa rough_claimed
  void set_rough_claimed(bool value) { rough_claimed_ = value; }

  /// \brief If true, it is reasonable to assume that this token is
  /// claimed by the current analysis. If false, more investigation
  /// may be required.
  bool rough_claimed() const { return rough_claimed_; }

  /// \sa language_independent
  void set_language_independent(bool value) { language_independent_ = value; }

  /// \brief If true, this node isn't language-specific.
  bool language_independent() const { return language_independent_; }

  bool operator==(const ClaimToken& rhs) const override {
    if (this == &rhs) {
      return true;
    }
    if (const auto* kythe_rhs = clang::dyn_cast<KytheClaimToken>(&rhs)) {
      return kythe_rhs->rough_claimed_ == rough_claimed_ &&
             kythe_rhs->vname_.corpus() == vname_.corpus() &&
             kythe_rhs->vname_.path() == vname_.path() &&
             kythe_rhs->vname_.root() == vname_.root();
    }
    return false;
  }

  bool operator!=(const ClaimToken& rhs) const override {
    if (const auto* kythe_rhs = clang::dyn_cast<KytheClaimToken>(&rhs)) {
      return kythe_rhs->rough_claimed_ != rough_claimed_ ||
             kythe_rhs->vname_.corpus() != vname_.corpus() ||
             kythe_rhs->vname_.path() != vname_.path() ||
             kythe_rhs->vname_.root() != vname_.root();
    }
    return true;
  }

 private:
  static inline const uintptr_t kTokenClass =
      reinterpret_cast<uintptr_t>(&kTokenClass);

  /// The prototypical VName to use for claimed objects.
  kythe::proto::VName vname_;
  bool rough_claimed_ = true;
  bool language_independent_ = false;
};

struct KytheGraphObserverOptions {
  // The custom build_config, if any, to apply to anchor nodes.
  std::string build_config = "";
  // The default corpus to use for nodes which would otherwise have an empty
  // corpus.
  std::string default_corpus = "";
  // Associates a hash to its semantic signature.
  HashRecorder* hash_recorder;
  // Use the default corpus for USRs?
  bool usr_default_corpus = false;
};

/// \brief Records details in the form of Kythe nodes and edges about elements
/// discovered during indexing to the provided `KytheGraphRecorder`.
///
/// \warning This class should not be used by multiple threads.
class KytheGraphObserver : public GraphObserver {
 public:
  using Options = KytheGraphObserverOptions;

  explicit KytheGraphObserver(KytheGraphRecorder* recorder,
                              KytheClaimClient* client,
                              const MetadataSupports* meta_supports,
                              const llvm::IntrusiveRefCntPtr<IndexVFS>& vfs,
                              ProfilingCallback ReportProfileEventCallback,
                              Options& options)
      : recorder_(ABSL_DIE_IF_NULL(recorder)),
        client_(ABSL_DIE_IF_NULL(client)),
        meta_supports_(ABSL_DIE_IF_NULL(meta_supports)),
        vfs_(vfs),
        build_config_(options.build_config),
        usr_default_corpus_(options.usr_default_corpus) {
    default_token_.set_rough_claimed(true);
    set_default_corpus(options.default_corpus);
    type_token_.set_rough_claimed(true);
    vname_token_.set_rough_claimed(true);
    ReportProfileEvent = std::move(ReportProfileEventCallback);
    RegisterBuiltins();
    EmitMetaNodes();
    hash_recorder_ = options.hash_recorder;
  }

  NodeId getNodeIdForBuiltinType(llvm::StringRef spelling) const override;

  const KytheClaimToken* getDefaultClaimToken() const override {
    return &default_token_;
  }

  const KytheClaimToken* getVNameClaimToken() const override {
    return &vname_token_;
  }

  void applyMetadataFile(clang::FileID ID, const clang::FileEntry* file,
                         const std::string& search_string,
                         const clang::FileEntry* target_file) override;
  void StopDeferringNodes() { deferring_nodes_ = false; }
  void DropRedundantWraiths() { drop_redundant_wraiths_ = true; }
  void Delimit() override { recorder_->PushEntryGroup(); }
  void Undelimit() override { recorder_->PopEntryGroup(); }

  NodeId nodeIdForTappNode(const NodeId& tycon_id,
                           absl::Span<const NodeId> params) const override;

  NodeId recordTappNode(const NodeId& tapp_id, const NodeId& tycon_id,
                        absl::Span<const NodeId> params,
                        unsigned first_default_param) override;

  NodeId nodeIdForTsigmaNode(absl::Span<const NodeId> params) const override;
  NodeId recordTsigmaNode(const NodeId& tsigma_id,
                          absl::Span<const NodeId> params) override;

  NodeId nodeIdForTypeAliasNode(const NameId& alias_name,
                                const NodeId& aliased_type) const override;

  NodeId recordTypeAliasNode(
      const NodeId& type_id, const NodeId& aliased_type,
      const std::optional<NodeId>& root_aliased_type,
      const std::optional<MarkedSource>& marked_source) override;

  void recordFunctionNode(
      const NodeId& node, Completeness function_completeness,
      FunctionSubkind subkind,
      const std::optional<MarkedSource>& marked_source) override;

  void assignUsr(const NodeId& node, llvm::StringRef usr,
                 int byte_size) override;

  void recordTVarNode(
      const NodeId& node,
      const std::optional<MarkedSource>& marked_source) override;

  void recordMarkedSource(
      const NodeId& node,
      const std::optional<MarkedSource>& marked_source) override;

  void recordLookupNode(const NodeId& node, llvm::StringRef text) override;

  void recordParamEdge(const NodeId& param_of_id, uint32_t ordinal,
                       const NodeId& param_id) override;

  void recordTParamEdge(const NodeId& param_of_id, uint32_t ordinal,
                        const NodeId& param_id) override;

  void recordInterfaceNode(
      const NodeId& node,
      const std::optional<MarkedSource>& marked_source) override;

  void recordRecordNode(
      const NodeId& node, RecordKind Kind, Completeness record_completeness,
      const std::optional<MarkedSource>& marked_source) override;

  void recordEnumNode(const NodeId& node, Completeness completeness,
                      EnumKind kind) override;

  void recordIntegerConstantNode(const NodeId& node,
                                 const llvm::APSInt& value) override;

  NodeId nodeIdForNominalTypeNode(const NameId& name_id) const override;

  NodeId recordNominalTypeNode(const NodeId& name_id,
                               const std::optional<MarkedSource>& marked_source,
                               const std::optional<NodeId>& parent) override;

  void recordCategoryExtendsEdge(const NodeId& from, const NodeId& to) override;

  void recordExtendsEdge(const NodeId& from, const NodeId& to, bool is_virtual,
                         clang::AccessSpecifier access_specifier) override;

  void recordDeclUseLocation(const Range& source_range, const NodeId& node,
                             GraphObserver::Claimability cl,
                             GraphObserver::Implicit i) override;

  void recordBlameLocation(const Range& source_range, const NodeId& blame,
                           GraphObserver::Claimability cl,
                           GraphObserver::Implicit i) override;

  void recordSemanticDeclUseLocation(const Range& SourceRange,
                                     const NodeId& DeclId, UseKind K,
                                     Claimability Cl, Implicit I) override;

  void recordInitLocation(const Range& source_range, const NodeId& node,
                          GraphObserver::Claimability cl,
                          GraphObserver::Implicit i) override;

  void recordVariableNode(
      const NodeId& decl_node, Completeness var_completeness,
      VariableSubkind subkind,
      const std::optional<MarkedSource>& marked_source) override;

  void recordNamespaceNode(
      const NodeId& decl_node,
      const std::optional<MarkedSource>& marked_source) override;

  void recordUserDefinedNode(const NodeId& node, llvm::StringRef node_kind,
                             std::optional<Completeness> completeness) override;

  void recordFullDefinitionRange(
      const Range& source_range, const NodeId& node_decl,
      const std::optional<NodeId>& node_def) override;

  void recordDefinitionBindingRange(
      const Range& binding_range, const NodeId& node_decl,
      const std::optional<NodeId>& node_def,
      Stamping stamping = Stamping::Stamped) override;

  void recordDefinitionRangeWithBinding(
      const Range& source_range, const Range& binding_range,
      const NodeId& node_decl, const std::optional<NodeId>& node_def) override;

  void recordDocumentationRange(const Range& source_range,
                                const NodeId& node) override;

  void recordDocumentationText(const NodeId& node, const std::string& doc_text,
                               const std::vector<NodeId>& doc_links) override;

  void recordDeclUseLocationInDocumentation(const Range& source_range,
                                            const NodeId& node) override;

  void recordCompletion(const NodeId& node,
                        const NodeId& completing_node) override;

  void recordTypeSpellingLocation(const Range& source_range,
                                  const NodeId& type_id,
                                  Claimability claimability,
                                  Implicit i) override;

  void recordTypeIdSpellingLocation(const Range& source_range,
                                    const NodeId& type_id,
                                    Claimability claimability,
                                    Implicit i) override;

  void recordChildOfEdge(const NodeId& child_id,
                         const NodeId& parent_id) override;

  void recordTypeEdge(const NodeId& term_id, const NodeId& type_id) override;

  void recordInfluences(const NodeId& influencer,
                        const NodeId& influenced) override;

  void recordUpperBoundEdge(const NodeId& TypeNodeId,
                            const NodeId& TypeBoundNodeId) override;

  void recordVariance(const NodeId& TypeNodeId,
                      const Variance variance) override;

  void recordSpecEdge(const NodeId& term_id, const NodeId& type_id,
                      Confidence conf) override;

  void recordInstEdge(const NodeId& term_id, const NodeId& type_id,
                      Confidence conf) override;

  void recordOverridesEdge(const NodeId& overrider,
                           const NodeId& base_object) override;

  void recordOverridesRootEdge(const NodeId& overrider,
                               const NodeId& root_object) override;

  void recordCallEdge(const Range& source_range, const NodeId& caller_id,
                      const NodeId& callee_id, Implicit i,
                      CallDispatch d) override;

  std::optional<NodeId> recordFileInitializer(const Range& range) override;

  void recordMacroNode(const NodeId& macro_id) override;

  void recordExpandsRange(const Range& source_range,
                          const NodeId& macro_id) override;

  void recordIndirectlyExpandsRange(const Range& source_range,
                                    const NodeId& macro_id) override;

  void recordUndefinesRange(const Range& source_range,
                            const NodeId& macro_id) override;

  void recordIncludesRange(const Range& source_range,
                           const clang::FileEntry* file) override;

  void recordBoundQueryRange(const Range& source_range,
                             const NodeId& macro_id) override;

  void recordStaticVariable(const NodeId& VarNodeId) override;

  void recordVisibility(const NodeId& FieldNodeId,
                        clang::AccessSpecifier access) override;

  void recordDeprecated(const NodeId& NodeId, llvm::StringRef advice) override;

  void recordDiagnostic(const Range& Range, llvm::StringRef Signature,
                        llvm::StringRef Message) override;

  void pushFile(clang::SourceLocation blame_location,
                clang::SourceLocation location) override;

  void popFile() override;

  bool isMainSourceFileRelatedLocation(
      clang::SourceLocation location) const override;

  void AppendMainSourceFileIdentifierToStream(
      llvm::raw_ostream& ostream) const override;

  /// \brief Configures the claimant that will be used to make claims.
  void set_claimant(const kythe::proto::VName& vname) { claimant_ = vname; }

  void set_default_corpus(absl::string_view corpus) {
    default_token_.mutable_vname()->set_corpus(std::string(corpus));
    type_token_.mutable_vname()->set_corpus(std::string(corpus));
  }

  bool claimNode(const NodeId& node_id) override {
    if (const auto* token =
            clang::dyn_cast<KytheClaimToken>(node_id.getToken())) {
      return token->rough_claimed();
    }
    return true;
  }

  bool claimRange(const GraphObserver::Range& range) override;

  bool claimLocation(clang::SourceLocation source_location) override;

  /// A representation of the state of the preprocessor.
  using PreprocessorContext = std::string;

  /// \brief Adds a fact about preprocessor contexts.
  /// \param path The (Clang-specific) lookup path for the file.
  /// \param context The source context.
  /// \param offset The offset into the source file.
  /// \param dest_context The context we'll transition into.
  void AddContextInformation(const std::string& path,
                             const PreprocessorContext& context,
                             unsigned offset,
                             const PreprocessorContext& dest_context);

  /// \brief Configures the starting context.
  /// \param context Context to use when the main source file is entered.
  void set_starting_context(const PreprocessorContext& context) {
    starting_context_ = context;
  }

  const KytheClaimToken* getClaimTokenForLocation(
      const clang::SourceLocation source_location) const override;

  const KytheClaimToken* getClaimTokenForRange(
      const clang::SourceRange& source_range) const override;

  const KytheClaimToken* getNamespaceClaimToken(
      clang::SourceLocation loc) const override;

  const KytheClaimToken* getAnonymousNamespaceClaimToken(
      clang::SourceLocation loc) const override;

  /// \brief Appends a representation of `Range` to `Ostream`.
  void AppendRangeToStream(llvm::raw_ostream& ostream,
                           const Range& range) const override;

  bool claimImplicitNode(const std::string& identifier) override;

  void finishImplicitNode(const std::string& identifier) override;

  bool claimBatch(std::vector<std::pair<std::string, bool>>* pairs) override;

  void iterateOverClaimedFiles(
      std::function<bool(clang::FileID, const NodeId&)> iter) const override;

  absl::string_view getBuildConfig() const override { return build_config_; }

  std::vector<std::pair<clang::FileID, const MetadataFile*>> GetMetadataFiles()
      const override {
    std::vector<std::pair<clang::FileID, const MetadataFile*>> files;
    for (const auto& meta : meta_) {
      files.push_back({meta.first, meta.second.get()});
    }
    return files;
  }

 private:
  /// A pair of tokens to use for namespaces.
  struct NamespaceTokens {
    KytheClaimToken named;      ///< Token to use for named namespaces.
    KytheClaimToken anonymous;  ///< Token to use for anonymous namespaces.
  };

  const NamespaceTokens& getNamespaceTokens(clang::SourceLocation loc) const;

  void AddMarkedSource(const VNameRef& vname,
                       const std::optional<MarkedSource>& signature) {
    if (signature) {
      recorder_->AddMarkedSource(vname, *signature);
    }
  }

  void RecordSourceLocation(const VNameRef& vname,
                            clang::SourceLocation source_location,
                            PropertyID offset_id);

  /// \brief Called by `AppendRangeToStream` to recur down a `SourceLocation`.
  ///
  /// A `SourceLocation` may have additional structure due to macro expansions.
  /// This function is used to generate a full serialization of this structure.
  void AppendFullLocationToStream(std::vector<clang::FileID>* posted_fileids,
                                  clang::SourceLocation loc,
                                  llvm::raw_ostream& Ostream) const;

  /// \brief Append a stable representation of `loc` to `Ostream`, even if
  /// `loc` is in a temporary buffer.
  void AppendFileBufferSliceHashToStream(clang::SourceLocation loc,
                                         llvm::raw_ostream& Ostream) const;

  VNameRef VNameRefFromNodeId(const GraphObserver::NodeId& node_id) const;
  kythe::proto::VName VNameFromFileEntry(
      const clang::FileEntry* file_entry) const;
  kythe::proto::VName ClaimableVNameFromFileID(const clang::FileID& file_id);
  kythe::proto::VName VNameFromRange(const GraphObserver::Range& range);
  kythe::proto::VName StampedVNameFromRange(const GraphObserver::Range& range,
                                            const GraphObserver::NodeId& stamp);
  /// Checks whether the edge from the anchor with `anchor_vname` covering
  /// `source_range` to `primary_anchored_to_decl` with kind `anchor_edge_kind`
  /// is interesting to any metadata provider.
  void ApplyMetadataRules(
      const GraphObserver::Range& source_range,
      const GraphObserver::NodeId& primary_anchored_to_decl,
      const std::optional<GraphObserver::NodeId>& primary_anchored_to_def,
      EdgeKindID anchor_edge_kind, const kythe::proto::VName& anchor_name);
  void RecordStampedAnchor(
      const GraphObserver::Range& source_range,
      const GraphObserver::NodeId& primary_anchored_to_decl,
      const std::optional<GraphObserver::NodeId>& primary_anchored_to_def,
      EdgeKindID anchor_edge_kind, const GraphObserver::NodeId& stamp);
  void RecordAnchor(const GraphObserver::Range& source_range,
                    const GraphObserver::NodeId& primary_anchored_to,
                    EdgeKindID anchor_edge_kind, Claimability claimability);
  void RecordAnchor(const GraphObserver::Range& source_range,
                    const kythe::proto::VName& primary_anchored_to,
                    EdgeKindID anchor_edge_kind, Claimability claimability);
  /// Records a Range.
  void RecordRange(const proto::VName& anchor_name,
                   const GraphObserver::Range& range);
  void UnconditionalRecordRange(const proto::VName& anchor_name,
                                const GraphObserver::Range& range);
  /// Execute metadata actions for `defines` edges.
  void MetaHookDefines(const MetadataFile& meta, const VNameRef& anchor,
                       unsigned range_begin, unsigned range_end,
                       const VNameRef& decl);

  /// Mark that file-scope rules are emitted for a given file.
  bool MarkFileMetaEdgeEmitted(const VNameRef& file_decl,
                               const MetadataFile& meta);

  struct RangeHash {
    size_t operator()(const GraphObserver::Range& range) const {
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

  struct StampedRangeHash {
    size_t operator()(const std::pair<Range, GraphObserver::NodeId>& p) const {
      return (std::hash<std::string>()(p.second.getRawIdentity())) << 1 ^
             RangeHash()(p.first);
    }
  };

  struct RangeEdge {
    clang::SourceRange PhysicalRange;
    EdgeKindID EdgeKind;
    GraphObserver::NodeId EdgeTarget;
    size_t Hash;
    bool operator==(const RangeEdge& range) const {
      return std::tie(Hash, PhysicalRange, EdgeKind, EdgeTarget) ==
             std::tie(range.Hash, range.PhysicalRange, range.EdgeKind,
                      range.EdgeTarget);
    }
    static size_t ComputeHash(const clang::SourceRange& PhysicalRange,
                              EdgeKindID EdgeKind,
                              const GraphObserver::NodeId& EdgeTarget) {
      return std::hash<unsigned>()(PhysicalRange.getBegin().getRawEncoding()) ^
             (std::hash<unsigned>()(PhysicalRange.getEnd().getRawEncoding())
              << 1) ^
             (std::hash<unsigned>()(static_cast<unsigned>(EdgeKind))) ^
             (std::hash<std::string>()(EdgeTarget.getRawIdentity()));
    }
  };

  struct ContextFreeRangeEdgeHash {
    size_t operator()(const RangeEdge& range) const { return range.Hash; }
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
  /// The metadata file ids for which we have already emitted file metadata.
  absl::flat_hash_set<
      std::tuple<std::string, std::string, std::string, std::string>>
      file_meta_edges_emitted_;
  /// All files that were ever reached through a header file, including header
  /// files themselves.
  std::set<llvm::sys::fs::UniqueID> transitively_reached_through_header_;
  /// A location in the main source file.
  clang::SourceLocation main_source_file_loc_;
  /// A claim token in the main source file.
  KytheClaimToken* main_source_file_token_ = nullptr;
  /// Files we have previously inspected for claiming. When they refer to
  /// FileEntries, a FileID represents a specific file being included from a
  /// given include position. There will therefore be many FileIDs that map to
  /// one context + header pair; then, many context + header pairs may
  /// map to a single file's VName.
  mutable std::map<clang::FileID, KytheClaimToken> claim_checked_files_;
  /// Tokens for files (independent of language) that we've claimed.
  std::map<clang::FileID, KytheClaimToken> claimed_file_specific_tokens_;
  /// Maps from claim tokens to claim tokens with path and root dropped.
  /// Used from logically const member funtions.
  mutable std::map<const KytheClaimToken*, NamespaceTokens> namespace_tokens_;
  /// The `KytheGraphRecorder` used to record graph data. Must not be null.
  KytheGraphRecorder* recorder_;
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
  KytheClaimClient* client_;
  /// Contains the `FileEntry`s for files we have already recorded.
  /// These pointers are not owned by the `KytheGraphObserver`.
  std::unordered_set<const clang::FileEntry*> recorded_files_;
  /// The set of anchor nodes with locations that have not yet been recorded.
  /// This allows the `GraphObserver` to limit the amount of redundant range
  /// information it emits should an anchor be the source of multiple edges.
  std::unordered_set<Range, RangeHash> deferred_anchors_;
  /// A set of (source range, edge kind, target node) tuples, used if
  /// drop_redundant_wraiths_ is asserted.
  std::unordered_set<RangeEdge, ContextFreeRangeEdgeHash> range_edges_;
  /// Used to eliminate redundant stamped ranges.
  std::unordered_set<std::pair<Range, GraphObserver::NodeId>, StampedRangeHash>
      stamped_ranges_;
  /// Used to eliminate redundant file-level initializers.
  std::unordered_set<const KytheClaimToken*> recorded_inits_;
  /// If true, anchors and edges from a given physical source location will
  /// be dropped if they were previously emitted from the same location
  /// with the same edge kind to the same target.
  bool drop_redundant_wraiths_ = false;
  /// Favor extra memory use during indexing over storing potentially redundant
  /// facts for certain frequently-used node kinds. Since these node kinds
  /// are defined to have structure equivalent to their names (modulo
  /// non-primary types in case of aliases, which may still be stored
  /// redundantly), this will not obscure conflicting-fact errors.
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
  const MetadataSupports* const meta_supports_;
  /// The virtual filesystem in use.
  llvm::IntrusiveRefCntPtr<IndexVFS> vfs_;
  /// A neutral claim token.
  KytheClaimToken default_token_;
  /// The claim token to use for structural types.
  KytheClaimToken type_token_;
  /// The claim token to use for encoded VNames.
  KytheClaimToken vname_token_;
  /// Name of the platform or build configuration to emit on anchors.
  const std::string build_config_;
  /// Information about builtin nodes.
  struct Builtin {
    /// This Builtin's NodeId.
    NodeId node_id;
    /// Marked source for this Builtin.
    MarkedSource marked_source;
    /// Whether this Builtin has been emitted.
    bool emitted;
  };
  /// Add known Builtins to builtins_.
  void RegisterBuiltins();
  /// Emit a particular Builtin.
  void EmitBuiltin(Builtin* builtin) const;
  /// Emit entries for C++ meta nodes.
  void EmitMetaNodes();
  /// Registered builtins.
  /// Modified lazily in const member functions.
  mutable std::map<std::string, Builtin> builtins_;
  // Use the default corpus for USRs?
  bool usr_default_corpus_ = false;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_KYTHE_GRAPH_OBSERVER_H_
