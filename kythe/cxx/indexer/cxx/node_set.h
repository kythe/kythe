/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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
#ifndef KYTHE_CXX_INDEXER_CXX_NODE_SET_H_
#define KYTHE_CXX_INDEXER_CXX_NODE_SET_H_

#include <optional>
#include <utility>

#include "kythe/cxx/indexer/cxx/GraphObserver.h"

namespace kythe {
// Contains the edge-connected set of possible NodeId's for a given entity.
// In particular, this is used to differentiate between tnominal nodes
// and a particular declaration when constructing references in
// accordance with:
// https://kythe.io/docs/schema#_references_to_definitions_and_declarations_of_types
class NodeSet {
  using Claimability = GraphObserver::Claimability;
  using NodeId = GraphObserver::NodeId;

 public:
  // Named constructor for a default-constructed (empty) NodeSet.
  static NodeSet Empty() { return {}; }

  NodeSet() = default;
  NodeSet(NodeId primary) : primary_(std::move(primary)) {}
  NodeSet(NodeId primary, Claimability claim)
      : claim_(claim), primary_(std::move(primary)) {}
  NodeSet(NodeId primary, NodeId secondary)
      : primary_(std::move(primary)), secondary_(std::move(secondary)) {}

  // NodeSet is both copyable and movable.
  NodeSet(const NodeSet&) = default;
  NodeSet& operator=(const NodeSet&) = default;
  NodeSet(NodeSet&&) = default;
  NodeSet& operator=(NodeSet&&) = default;

  bool has_value() const { return primary_.has_value(); }
  explicit operator bool() const { return has_value(); }
  const NodeId& value() const& { return primary_.value(); }
  NodeId&& value() && { return std::move(primary_).value(); }
  const NodeId& operator*() const& { return *primary_; }
  NodeId&& operator*() && { return *std::move(primary_); }
  const NodeId* operator->() const { return primary_.operator->(); }

  /// \brief When emitting a "ref" edge to this entity, prefer this node.
  /// This is primarily used to prefer a particular declaration in situations
  /// where we oridinarily want to use a tnomical node.
  const NodeId& ForReference() const {
    return (secondary_.has_value() ? *secondary_ : *primary_);
  }

  const std::optional<NodeId>& AsOptional() const& { return primary_; }
  std::optional<NodeId>&& AsOptional() && { return std::move(primary_); }

  Claimability claimability() const { return claim_; }

 private:
  Claimability claim_ = Claimability::Claimable;
  std::optional<NodeId> primary_;
  std::optional<NodeId> secondary_;
};
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_NODE_SET_H_
