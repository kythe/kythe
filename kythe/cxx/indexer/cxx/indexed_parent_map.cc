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
#include "indexed_parent_map.h"

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "kythe/cxx/common/scope_guard.h"

namespace kythe {
namespace {

using ::clang::dyn_cast;

/// \return `true` if truncating tree traversal at `decl` is safe, provided that
/// `decl` has been traversed previously.
bool IsClaimableForTraverse(const clang::Decl* decl) {
  // Operationally, we'll define this as any decl that causes
  // Job->UnderneathImplicitTemplateInstantiation to be set.
  if (auto* vtsd = dyn_cast<const clang::VarTemplateSpecializationDecl>(decl)) {
    return !vtsd->isExplicitInstantiationOrSpecialization();
  }
  if (auto* ctsd =
          dyn_cast<const clang::ClassTemplateSpecializationDecl>(decl)) {
    return !ctsd->isExplicitInstantiationOrSpecialization();
  }
  if (auto* fndecl = dyn_cast<const clang::FunctionDecl>(decl)) {
    if (const auto* msi = fndecl->getMemberSpecializationInfo()) {
      // The definitions of class template member functions are not necessarily
      // dominated by the class template definition.
      if (!msi->isExplicitSpecialization()) {
        return true;
      }
    } else if (const auto* fsi = fndecl->getTemplateSpecializationInfo()) {
      if (!fsi->isExplicitInstantiationOrSpecialization()) {
        return true;
      }
    }
  }
  return false;
}

/// \return `true` if truncating tree traversal at `stmt` is safe, provided that
/// `stmt` has been traversed previously.
bool IsClaimableForTraverse(const clang::Stmt* S) { return false; }

template <typename MappingType>
class IndexedParentASTVisitor
    : private clang::RecursiveASTVisitor<IndexedParentASTVisitor<MappingType>> {
 public:
  static MappingType BuildIndexedParentMap(clang::TranslationUnitDecl* unit) {
    IndexedParentASTVisitor visitor;
    visitor.TraverseDecl(unit);
    return std::move(visitor.parents_);
  }

 private:
  using VisitorBase = clang::RecursiveASTVisitor<IndexedParentASTVisitor>;
  friend class clang::RecursiveASTVisitor<IndexedParentASTVisitor>;

  bool shouldVisitTemplateInstantiations() const { return true; }
  bool shouldVisitImplicitCode() const { return true; }
  // Disables data recursion. We intercept Traverse* methods in the RAV, which
  // are not triggered during data recursion.
  bool shouldUseDataRecursionFor(clang::Stmt* stmt) const { return false; }

  // Traverse an arbitrary AST node type and record the node used to get to
  // it as that node's parent. `T` is the type of the node and
  // `BaseTraverseFn` is the type of a function (or other value with
  // an operator()) that invokes the base RecursiveASTVisitor traversal logic
  // on values of type `T*` and returns a boolean traversal result.
  template <typename T, typename BaseTraverseFn>
  bool TraverseNode(T* node, BaseTraverseFn traverse) {
    using ::clang::DynTypedNode;
    if (node == nullptr) return true;
    if (!parent_stack_.empty()) {
      auto& parent = parents_[node];
      if (parent.getPointer() == nullptr) {
        parent.setPointer(new IndexedParent(parent_stack_.back()));
      }
    }

    bool saved_claimable = claimable_at_this_depth_;
    auto scope = MakeScopeGuard([&] {
      if (claimable_at_this_depth_ || IsClaimableForTraverse(node)) {
        claimable_at_this_depth_ = true;  // for depth
        parents_[node].setInt(1);
      } else {
        claimable_at_this_depth_ = saved_claimable;
      }
      parent_stack_.pop_back();
      if (!parent_stack_.empty()) {
        parent_stack_.back().index++;
      }
    });

    parent_stack_.push_back({DynTypedNode::create(*node), 0});
    claimable_at_this_depth_ = false;  // for depth + 1
    return traverse(node);
    // `scope` executes.
  }

  bool TraverseDecl(clang::Decl* decl) {
    return TraverseNode(decl, [this](clang::Decl* node) {
      return VisitorBase::TraverseDecl(node);
    });
  }

  bool TraverseStmt(clang::Stmt* stmt) {
    return TraverseNode(stmt, [this](clang::Stmt* node) {
      return VisitorBase::TraverseStmt(node);
    });
  }

  MappingType parents_;
  llvm::SmallVector<IndexedParent, 16> parent_stack_;
  bool claimable_at_this_depth_ = false;
};
}  // namespace

/* static */
IndexedParentMap IndexedParentMap::Build(clang::TranslationUnitDecl* unit) {
  return IndexedParentMap(
      IndexedParentASTVisitor<MappingType>::BuildIndexedParentMap(unit));
}

const IndexedParent* IndexedParentMap::GetIndexedParent(
    const clang::DynTypedNode& node) const {
  CHECK(node.getMemoizationData() != nullptr)
      << "Invariant broken: only nodes that support memoization may be "
         "used in the parent map.";
  const auto iter = parents_.find(node.getMemoizationData());
  if (iter == parents_.end()) {
    return nullptr;
  }
  return iter->second.getPointer();
}

bool IndexedParentMap::DeclDominatesPrunableSubtree(
    const clang::Decl* decl) const {
  const auto node = clang::DynTypedNode::create(*decl);
  const auto iter = parents_.find(node.getMemoizationData());
  if (iter == parents_.end()) {
    return false;  // Safe default.
  }
  return iter->second.getInt() == 0;
}

}  // namespace kythe
