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
#include "KytheGraphRecorder.h"
#include "kythe/proto/storage.pb.h"

namespace kythe {

class KytheGraphRecorder;

/// \brief Records details in the form of Kythe nodes and edges about elements
/// discovered during indexing to the provided `KytheGraphRecorder`.
class KytheGraphObserver : public GraphObserver {
 public:
  KytheGraphObserver(KytheGraphRecorder *recorder) : recorder_(recorder) {
    assert(recorder_ != nullptr);
  }

  NodeId getNodeIdForBuiltinType(const llvm::StringRef &spelling) override {
    NodeId id;
    id.Signature = spelling.str();
    id.Signature.append("#builtin");
    return id;
  }

  NodeId recordTappNode(const NodeId &TyconId,
                        const std::vector<const NodeId *> &Params) override;

  NodeId nodeIdForTypeAliasNode(const NameId &AliasName,
                                const NodeId &AliasedType) override;

  NodeId recordTypeAliasNode(const NameId &DeclName,
                             const NodeId &DeclNode) override;

  void recordFunctionNode(const NodeId &Node,
                          Completeness FunctionCompleteness);

  void recordCallableNode(const NodeId &Node);

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

  void recordDeclUseLocation(const Range &SourceRange,
                             const NodeId &DeclId) override;

  void recordVariableNode(const NameId &DeclName, const NodeId &DeclNode,
                          Completeness VarCompleteness) override;

  void recordDefinitionRange(const Range &SourceRange,
                             const NodeId &DefnId) override;

  void recordCompletionRange(const Range &SourceRange, const NodeId &DefnId,
                             Specificity Spec) override;

  void recordNamedEdge(const NodeId &Node, const NameId &Name) override;

  void recordTypeSpellingLocation(const Range &SourceRange,
                                  const NodeId &TypeId) override;

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

  void recordUndefinesRange(const Range &SourceRange,
                            const NodeId &MacroId) override;

  void recordIncludesRange(const Range &SourceRange,
                           const clang::FileEntry *File) override;

  void recordBoundQueryRange(const Range &SourceRange,
                             const NodeId &MacroId) override;

  void recordUnboundQueryRange(const Range &SourceRange,
                               const NameId &MacroName) override;

  void pushFile(clang::SourceLocation loc) override;

  void popFile() override;

  /// \brief Associates a path with a vname.
  /// \param path The path to associate. This should be the Clang lookup path.
  /// \param vname The vname to use for this path.
  void set_path_vname(const std::string &path,
                      const kythe::proto::VName &vname) {
    path_to_vname_[path] = vname;
  }

 private:
  void RecordSourceLocation(clang::SourceLocation source_location,
                            PropertyID offset_id);
  kythe::proto::VName VNameFromNodeId(const GraphObserver::NodeId &node_id);
  kythe::proto::VName VNameFromFileID(const clang::FileID &file_id);
  kythe::proto::VName VNameFromFileEntry(const clang::FileEntry *file_entry);
  kythe::proto::VName VNameFromRange(const GraphObserver::Range &range);
  kythe::proto::VName RecordName(const GraphObserver::NameId &name_id);
  kythe::proto::VName RecordAnchor(
      const GraphObserver::Range &source_range,
      const kythe::proto::VName &primary_anchored_to,
      EdgeKindID anchor_edge_kind);
  /// Records any deferred nodes, clearing any records of their deferral.
  void RecordDeferredNodes();

  struct RangeHash {
    size_t operator()(const GraphObserver::Range &range) const {
      return std::hash<unsigned>()(
                 range.PhysicalRange.getBegin().getRawEncoding()) ^
             (std::hash<unsigned>()(
                  range.PhysicalRange.getEnd().getRawEncoding())
              << 1) ^
             (std::hash<std::string>()(range.Context.Signature)) ^
             (range.Kind == Range::RangeKind::Wraith ? 1 : 0);
    }
  };

  /// The number of files we have entered but not left.
  int file_stack_size_ = 0;
  /// If nonempty, this string is used to set the `root` of `VNames` that
  /// this `KytheGraphObserver` produces.
  std::string default_root_;
  /// If nonempty, this string is used to set the `corpus` of `VNames` that
  /// this `KytheGraphObserver` produces.
  std::string default_corpus_;
  /// The `KytheGraphRecorder` used to record graph data. Must not be null.
  KytheGraphRecorder *recorder_;
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
  /// The set of `talias` nodes we've emitted so far (identified by
  /// `NodeId::ToString()`).
  std::unordered_set<std::string> written_taliases_;
  /// The set of `tapp` nodes we've emitted so far (identified by
  /// `NodeId::ToString()`).
  std::unordered_set<std::string> written_tapps_;
  /// A map from paths to VNames, extracted from a .kindex file.
  std::unordered_map<std::string, kythe::proto::VName> path_to_vname_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_KYTHE_GRAPH_OBSERVER_H_
