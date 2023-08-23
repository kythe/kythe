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
#ifndef KYTHE_CXX_INDEXER_CXX_INDEXED_PARENT_ITERATOR_H_
#define KYTHE_CXX_INDEXER_CXX_INDEXED_PARENT_ITERATOR_H_

#include <iterator>
#include <optional>

#include "clang/AST/ASTTypeTraits.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"
#include "kythe/cxx/indexer/cxx/indexed_parent_map.h"

namespace kythe {

// Range class for a traversal from the given Decl to the root.
class RootTraversal {
 public:
  // Type containing the values used during root traversal iterator.
  struct Context {
    // The current node in the traversal.
    clang::DynTypedNode node;
    // A pointer to the `clang::Decl` of the node, if any.
    const clang::Decl* decl;
    // A pointer to the IndexedParent of the current node, if any.
    // This will be next current node.
    const IndexedParent* indexed_parent;

    // Convenience accessor for the traversal index of this node
    // within its parent, if any.
    std::optional<size_t> parent_index() const {
      if (indexed_parent == nullptr) {
        return std::nullopt;
      }
      return indexed_parent->index;
    }

    // The values are EqComparable and only care about node.
    bool operator==(const Context& rhs) const { return node == rhs.node; }
    bool operator!=(const Context& rhs) const { return !(*this == rhs); }
  };
  using value_type = Context;

  // Root iteration state, modeling standard iterators in all their awkwardness.
  class iterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Context;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;

    iterator() = default;
    reference operator*() const { return *current_; }
    pointer operator->() const { return &*current_; }
    bool operator==(const iterator& rhs) const;
    bool operator!=(const iterator& rhs) const;
    iterator& operator++();
    iterator operator++(int);

   private:
    friend class RootTraversal;

    explicit iterator(const IndexedParentMap* parent_map,
                      clang::DynTypedNode node, const clang::Decl* decl,
                      const IndexedParent* parent)
        : parent_map_(parent_map), current_(value_type{node, decl, parent}) {
      // If we would be constructed from a TranslationUnitDecl, stop.
      if (decl && clang::isa<clang::TranslationUnitDecl>(decl)) {
        parent_map_ = nullptr;
        current_ = std::nullopt;
      }
    }

    explicit iterator(const IndexedParentMap* parent_map,
                      clang::DynTypedNode node, const clang::Decl* decl)
        : iterator(parent_map, node, decl, parent_map->GetIndexedParent(node)) {
    }

    explicit iterator(const IndexedParentMap* parent_map,
                      clang::DynTypedNode node)
        : iterator(parent_map, node, node.get<clang::Decl>()) {}

    explicit iterator(const IndexedParentMap* parent_map,
                      const clang::Decl* decl)
        : iterator(parent_map, clang::DynTypedNode::create(*decl), decl) {}

    iterator next() const;
    void advance();

    const IndexedParentMap* parent_map_ = nullptr;
    std::optional<value_type> current_ = std::nullopt;
  };

  // Constructs a RootTraversal range over `parent_map` beginning at
  // the node indicated in `decl`.
  explicit RootTraversal(const IndexedParentMap* parent_map,
                         const clang::Decl* decl)
      : start_(Start(parent_map, decl)) {}

  // Constructs a RootTraversal range over `parent_map` beginning at
  // the node indicated in `stmt`.
  explicit RootTraversal(const IndexedParentMap* parent_map,
                         const clang::Stmt* stmt)
      : start_(Start(parent_map, stmt)) {}

  iterator begin() const { return start_; }
  iterator end() const { return iterator(); }

 private:
  static iterator Start(const IndexedParentMap* parent_map,
                        const clang::Decl* decl) {
    return decl ? iterator(parent_map, decl) : iterator();
  }
  static iterator Start(const IndexedParentMap* parent_map,
                        const clang::Stmt* stmt) {
    return stmt ? iterator(parent_map, clang::DynTypedNode::create(*stmt))
                : iterator();
  }
  iterator start_;
};

inline bool RootTraversal::iterator::operator==(const iterator& rhs) const {
  return std::tie(parent_map_, current_) ==
         std::tie(rhs.parent_map_, rhs.current_);
}

inline bool RootTraversal::iterator::operator!=(const iterator& rhs) const {
  return !(*this == rhs);
}

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_INDEXED_PARENT_ITERATOR_H_
