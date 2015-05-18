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

#ifndef KYTHE_CXX_INDEXER_CXX_KYTHE_GRAPH_RECORDER_H_
#define KYTHE_CXX_INDEXER_CXX_KYTHE_GRAPH_RECORDER_H_

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
  kCallable,
  kLookup,
  kMacro
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
  kSubkind
};

/// \brief Known edge kinds. See the schema for details.
enum class EdgeKindID {
  kDefines,
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
  kCallableAs,
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
  kExtendsVirtual
};

/// \brief Returns the Kythe spelling of `node_kind_id`
///
/// ~~~
/// spelling_of(kAnchor) == "/kythe/anchor"
/// ~~~
const std::string &spelling_of(NodeKindID node_kind_id);

/// \brief Returns the Kythe spelling of `property_id`
///
/// ~~~
/// spelling_of(kLocation) == "/kythe/loc/uri"
/// ~~~
const std::string &spelling_of(PropertyID property_id);

/// \brief Returns the Kythe spelling of `edge_kind_id`
///
/// ~~~
/// spelling_of(kDefines) == "/kythe/defines"
/// ~~~
const std::string &spelling_of(EdgeKindID edge_kind_id);

/// \brief Records Kythe nodes and edges to a provided `KytheOutputStream`.
class KytheGraphRecorder {
 public:
  /// A Kythe VName.
  typedef kythe::proto::VName VName;

  /// \brief Renders nodes and edges to the provided `KytheOutputStream`.
  /// \param stream The stream into which nodes and edges should be emitted.
  explicit KytheGraphRecorder(KytheOutputStream *stream) : stream_(stream) {
    assert(stream_ != nullptr);
  }

  /// \brief Begins recording information about a node.
  ///
  /// Only one node may be recorded at once and edges may not be recorded
  /// while a node is incomplete. Begin by calling `BeginNode`, then
  /// make zero or more calls to `AddProperty`, and finally conclude with a call
  /// to `EndNode`:
  ///
  /// ~~~
  /// recorder.BeginNode(vname, KytheGraphRecorder::kAnchor);
  /// recorder.AddProperty(KytheGraphRecorder::kLocationUri, "test://file");
  /// recorder.EndNode();
  /// ~~~
  ///
  /// \pre Any previous calls to `BeginNode` have been matched with `EndNode`.
  /// \sa AddProperty, EndNode
  void BeginNode(const VName &node_vname, NodeKindID kind_id);

  /// \brief Begins recording information about a node with a user-defined kind.
  /// \sa BeginNode
  void BeginNode(const VName &node_vname, const llvm::StringRef &kind_id);

  /// \brief Record a property about the current node.
  ///
  /// It is invalid to call `AddProperty` outside of a `BeginNode`...`EndNode`
  /// sequence.
  ///
  /// \param property_id The `PropertyID` of the property to record.
  /// \param property_value The value of the property to set.
  /// \pre `BeginNode` has been called, but a matching call to `EndNode` has not
  /// yet been made.
  /// \sa BeginNode, EndNode
  void AddProperty(PropertyID property_id, const std::string &property_value);

  /// \copydoc KytheGraphRecorder::AddProperty(PropertyID,std::string&)
  void AddProperty(PropertyID property_id, size_t property_value);

  /// \brief Commit the current partially-constructed node.
  /// \pre `BeginNode` has been called, but a matching call to `EndNode` has
  /// not yet been made.
  /// \sa BeginNode, AddProperty
  void EndNode();

  /// \brief Records an edge between nodes.
  ///
  /// It is invalid to call `AddEdge` in the middle of a `BeginNode`...`EndNode`
  /// sequence. `VName` may name a node that has not been previously described
  /// via `BeginNode`.
  ///
  /// \param edge_from The `VName` of the node at which the edge starts.
  /// \param edge_kind_id The `EdgeKindID` of the edge.
  /// \param edge_to The `VName` of the node at which the edge terminates.
  void AddEdge(const VName &edge_from, EdgeKindID edge_kind_id,
               const VName &edge_to);

  /// \brief Records an edge between nodes with an associated ordinal.
  ///
  /// It is invalid to call `AddEdge` in the middle of a `BeginNode`...`EndNode`
  /// sequence. `VName` may name a node that has not been previously described
  /// via `BeginNode`.
  ///
  /// \param edge_from The `VName` of the node at which the edge starts.
  /// \param edge_kind_id The `EdgeKindID` of the edge.
  /// \param edge_to The `VName` of the node at which the edge terminates.
  /// \param edge_ordinal The edge's associated ordinal.
  void AddEdge(const VName &edge_from, EdgeKindID edge_kind_id,
               const VName &edge_to, uint32_t edge_ordinal);

  /// \brief Records the content of a file that was visited during compilation.
  /// \param file_vname The file's vname.
  /// \param file_content The buffer of this file's content.
  void AddFileContent(const VName &file_vname,
                      const llvm::StringRef &file_content);

 private:
  /// The `KytheOutputStream` to which new graph elements are written.
  KytheOutputStream *stream_;
  /// The current incomplete node's VName. Meaningful if `in_node_ == true`.
  VName node_vname_;
  /// `true` if there is an uncommitted node.
  bool in_node_ = false;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_KYTHE_GRAPH_RECORDER_H_
