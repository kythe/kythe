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

#ifndef KYTHE_CXX_COMMON_INDEXING_KYTHE_GRAPH_RECORDER_H_
#define KYTHE_CXX_COMMON_INDEXING_KYTHE_GRAPH_RECORDER_H_

#include "llvm/ADT/StringRef.h"

#include "KytheOutputStream.h"

namespace kythe {

/// \brief Known node kinds. See the schema for details.
enum class NodeKindID {
  kAnchor,
  kFile,
  kVariable,
  kTAlias,
  kTApp,
  kTNominal,
  kRecord,
  kSum,
  kConstant,
  kAbs,
  kAbsVar,
  kName,
  kFunction,
  kLookup,
  kMacro,
  kInterface,
  kPackage,
  kTSigma,
  kDoc,
  kBuiltin,
  kMeta
};

/// \brief Known properties of nodes. See the schema for details.
enum class PropertyID {
  kLocation,
  kLocationUri,
  kLocationStart,
  kLocationStartRow,
  kLocationStartOffset,
  kLocationEnd,
  kLocationEndRow,
  kLocationEndOffset,
  kText,
  kComplete,
  kSubkind,
  kNodeKind,
  kFormat
};

/// \brief Known edge kinds. See the schema for details.
enum class EdgeKindID {
  kDefinesFull,
  kNamed,
  kHasType,
  kRef,
  kParam,
  kAliases,
  kUniquelyCompletes,
  kCompletes,
  kChildOf,
  kSpecializes,
  kRefCall,
  kRefExpands,
  kUndefines,
  kRefIncludes,
  kRefQueries,
  kInstantiates,
  kRefExpandsTransitive,
  kExtendsPublic,
  kExtendsProtected,
  kExtendsPrivate,
  kExtends,
  kExtendsPublicVirtual,
  kExtendsProtectedVirtual,
  kExtendsPrivateVirtual,
  kExtendsVirtual,
  kSpecializesSpeculative,
  kInstantiatesSpeculative,
  kDocuments,
  kRefDoc,
  kGenerates,
  kDefinesBinding,
  kOverrides,
  kChildOfContext
};

/// \brief Returns the Kythe spelling of `node_kind_id`
///
/// ~~~
/// spelling_of(kAnchor) == "/kythe/anchor"
/// ~~~
llvm::StringRef spelling_of(NodeKindID node_kind_id);

/// \brief Returns the Kythe spelling of `property_id`
///
/// ~~~
/// spelling_of(kLocation) == "/kythe/loc/uri"
/// ~~~
llvm::StringRef spelling_of(PropertyID property_id);

/// \brief Returns the Kythe spelling of `edge_kind_id`
///
/// ~~~
/// spelling_of(kDefines) == "/kythe/defines"
/// ~~~
llvm::StringRef spelling_of(EdgeKindID edge_kind_id);

/// Returns true and sets `out_edge` to the enumerator corresponding to
/// `spelling` (or returns false if there is no such correspondence).
bool of_spelling(llvm::StringRef spelling, EdgeKindID *out_edge);

/// \brief Records Kythe nodes and edges to a provided `KytheOutputStream`.
class KytheGraphRecorder {
 public:
  /// \brief Renders nodes and edges to the provided `KytheOutputStream`.
  /// \param stream The stream into which nodes and edges should be emitted.
  explicit KytheGraphRecorder(KytheOutputStream *stream) : stream_(stream) {
    assert(stream_ != nullptr);
  }

  /// \brief Record a property about a node.
  ///
  /// \param node_vname The vname of the node to modify.
  /// \param property_id The `PropertyID` of the property to record.
  /// \param property_value The value of the property to set.
  void AddProperty(const VNameRef &node_vname, PropertyID property_id,
                   const std::string &property_value);

  /// \copydoc KytheGraphRecorder::AddProperty(const
  /// VNameRef&,PropertyID,std::string&)
  void AddProperty(const VNameRef &node_vname, PropertyID property_id,
                   size_t property_value);

  /// \copydoc KytheGraphRecorder::AddProperty(const
  /// VNameRef&,PropertyID,std::string&)
  void AddProperty(const VNameRef &node_vname, NodeKindID node_kind_value) {
    AddProperty(node_vname, PropertyID::kNodeKind,
                spelling_of(node_kind_value));
  }

  /// \brief Records an edge between nodes.
  ///
  /// \param edge_from The `VNameRef` of the node at which the edge starts.
  /// \param edge_kind_id The `EdgeKindID` of the edge.
  /// \param edge_to The `VNameRef` of the node at which the edge terminates.
  void AddEdge(const VNameRef &edge_from, EdgeKindID edge_kind_id,
               const VNameRef &edge_to);

  /// \brief Records an edge between nodes with an associated ordinal.
  ///
  /// \param edge_from The `VNameRef` of the node at which the edge starts.
  /// \param edge_kind_id The `EdgeKindID` of the edge.
  /// \param edge_to The `VNameRef` of the node at which the edge terminates.
  /// \param edge_ordinal The edge's associated ordinal.
  void AddEdge(const VNameRef &edge_from, EdgeKindID edge_kind_id,
               const VNameRef &edge_to, uint32_t edge_ordinal);

  /// \brief Records the content of a file that was visited during compilation.
  /// \param file_vname The file's vname.
  /// \param file_content The buffer of this file's content.
  void AddFileContent(const VNameRef &file_vname,
                      const llvm::StringRef &file_content);

  /// \brief Stop using the last entry group pushed to the stack.
  void PopEntryGroup() { stream_->PopBuffer(); }

  /// \brief Push a new entry group to the group stack.
  ///
  /// Subsequent entries and groups will be attributed to this group.
  /// Various output stream policies determine when a group is ready to be
  /// released. Every PushEntryGroup should be paired with a PopEntryGroup.
  void PushEntryGroup() { stream_->PushBuffer(); }

 private:
  /// The `KytheOutputStream` to which new graph elements are written.
  KytheOutputStream *stream_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_INDEXING_KYTHE_GRAPH_RECORDER_H_
