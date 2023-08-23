/*
 * Copyright 2023 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_CXX_DECL_PRINTER_H_
#define KYTHE_CXX_INDEXER_CXX_DECL_PRINTER_H_

#include <string>

#include "absl/base/attributes.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclarationName.h"
#include "kythe/cxx/indexer/cxx/GraphObserver.h"
#include "kythe/cxx/indexer/cxx/indexed_parent_iterator.h"
#include "kythe/cxx/indexer/cxx/indexed_parent_map.h"
#include "kythe/cxx/indexer/cxx/semantic_hash.h"
#include "llvm/Support/raw_ostream.h"

namespace kythe {

/// \brief DeclPrinter formats declarations into unique names.
class DeclPrinter {
 public:
  explicit DeclPrinter(
      const GraphObserver& observer ABSL_ATTRIBUTE_LIFETIME_BOUND,
      const SemanticHash& hasher ABSL_ATTRIBUTE_LIFETIME_BOUND,
      const LazyIndexedParentMap& parent_map ABSL_ATTRIBUTE_LIFETIME_BOUND)
      : observer_(&observer), hasher_(&hasher), parent_map_(&parent_map) {}

  /// \brief Returns an identifer path as a string.
  std::string QualifiedId(const clang::Decl& decl) const;
  /// \brief Writes a representation of the full path to `out`.
  void PrintQualifiedId(const clang::Decl& decl, llvm::raw_ostream& out) const;

 private:
  void PrintQualifiedId(const RootTraversal& path,
                        llvm::raw_ostream& out) const;

  /// \brief Attemtps to add some representation of `decl` to `out`.
  /// \return true if the full name was written to `out`;
  ///         false indicates some data may have been written to `out`,
  ///         but does not constitute the full name.
  bool PrintName(const clang::Decl* decl, llvm::raw_ostream& out) const;
  bool PrintNamedDecl(const clang::NamedDecl& decl,
                      llvm::raw_ostream& out) const;

  /// \brief Attempts to add `name` to `out`.
  /// \return true if the full name was written to `out`;
  ///         false indicates some data may have been written to `out`,
  ///         but does not constitute the full name.
  bool PrintDeclarationName(const clang::DeclarationName& name,
                            llvm::raw_ostream& out) const;

  const GraphObserver& observer() const { return *observer_; }
  const SemanticHash& hasher() const { return *hasher_; }

  const GraphObserver* observer_;
  const SemanticHash* hasher_;
  const LazyIndexedParentMap* parent_map_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_DECL_PRINTER_H_
