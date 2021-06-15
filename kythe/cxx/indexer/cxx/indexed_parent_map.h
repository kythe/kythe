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
#ifndef KYTHE_CXX_INDEXER_CXX_INDEXED_PARENT_MAP_H_
#define KYTHE_CXX_INDEXER_CXX_INDEXED_PARENT_MAP_H_

#include "clang/AST/ASTTypeTraits.h"
#include "clang/AST/Decl.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PointerIntPair.h"

namespace kythe {

/// For a given node in the AST, this class keeps track of the node's
/// parent (along some path from the AST root) and an integer index for that
/// node in some arbitrary but consistent order defined by the parent.
struct IndexedParent {
  /// \brief The parent DynTypedNode associated with some key.
  clang::DynTypedNode parent;
  /// \brief The index at which some associated key appears in `Parent`.
  size_t index;

  friend bool operator==(const IndexedParent& lhs, const IndexedParent& rhs) {
    // We compare IndexedParents for deduplicating memoizable DynTypedNodes
    // below; semantically, this means that we keep the first child index
    // we saw when following every path through a particular memoizable
    // IndexedParent.
    return lhs.parent == rhs.parent;
  }

  friend bool operator!=(const IndexedParent& lhs, const IndexedParent& rhs) {
    return !(lhs == rhs);
  }
};

class IndexedParentMap {
 public:
  /// \brief Builds and returns the translation unit's indexed parent map.
  static IndexedParentMap Build(clang::TranslationUnitDecl* unit);

  // IndexedParentMap retains ownership of the contained IndexedParents and
  // thus is move-only.
  IndexedParentMap(IndexedParentMap&&) = default;
  IndexedParentMap& operator=(IndexedParentMap&&) = default;

  bool empty() const { return parents_.empty(); }

  /// \brief Returns the parent of the given node, along with the index
  /// at which the node appears underneath each parent.
  const IndexedParent* GetIndexedParent(const clang::DynTypedNode& node) const;

  /// \brief Returns the parent of the given node, along with the index
  /// at which the node appears underneath each parent.
  ///
  /// 'NodeT' can be one of Decl, Stmt, Type, TypeLoc,
  /// NestedNameSpecifier or NestedNameSpecifierLoc.
  template <typename T>
  const IndexedParent* GetIndexedParent(const T& node) const {
    return GetIndexedParent(clang::DynTypedNode::create(node));
  }

  /// \return true if `Decl` and all of the nodes underneath it are prunable.
  ///
  /// A subtree is prunable if it's "the same" in all possible indexer runs.
  /// This excludes, for example, certain template instantiations.
  bool DeclDominatesPrunableSubtree(const clang::Decl* decl) const;

 private:
  // An owned, move-only, PointerIntPair.
  struct PointerPair : llvm::PointerIntPair<IndexedParent*, 1, bool> {
    PointerPair() = default;

    PointerPair(PointerPair&& other)
        : PointerIntPair(other.getPointer(), other.getInt()) {
      other.setPointer(nullptr);
    }

    PointerPair& operator=(PointerPair&& other) {
      setPointerAndInt(other.getPointer(), other.getInt());
      other.setPointer(nullptr);
      return *this;
    }

    ~PointerPair() { delete PointerIntPair::getPointer(); }
  };
  using MappingType = llvm::DenseMap<const void*, PointerPair>;

  explicit IndexedParentMap(MappingType parents)
      : parents_(std::move(parents)) {}

  MappingType parents_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_INDEXED_PARENT_MAP_H_
