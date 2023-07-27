/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_CXX_MARKED_SOURCE_H_
#define KYTHE_CXX_INDEXER_CXX_MARKED_SOURCE_H_

#include <optional>

#include "clang/AST/Decl.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Sema/Sema.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/indexer/cxx/GraphObserver.h"
#include "llvm/ADT/DenseMap.h"

namespace kythe {
class MarkedSourceCache;

/// \brief Collects information about a `decl`, then possibly marks up its
/// source for presentation as a signature in documentation. Typically this
/// drops function and struct bodies while preserving the information in
/// prototypes.
class MarkedSourceGenerator {
 public:
  /// The decl in question is implicit. Inhibits marked source generation.
  void set_implicit(bool value) { implicit_ = value; }

  /// Sets the end of the marked source. By default, the beginning of the marked
  /// source is assumed to start at `decl->getSourceRange().getBegin()`; finding
  /// the end is decl type-specific.
  void set_marked_source_end(clang::SourceLocation loc) { end_loc_ = loc; }

  /// Sets the range that covers the decl's (unqualified) name.
  void set_name_range(const clang::SourceRange& range) { name_range_ = range; }

  /// \return true if GenerateMarkedSource will do work.
  /// \note This does not guarantee that GenerateMarkedSource != None.
  bool WillGenerateMarkedSource() const;

  /// Attempt to build a marked source given all available information. Assumes
  /// that `decl_`'s ID is `decl_id`.
  std::optional<MarkedSource> GenerateMarkedSource(
      const GraphObserver::NodeId& decl_id);

 private:
  friend class MarkedSourceCache;

  MarkedSourceGenerator(MarkedSourceCache* cache, const clang::NamedDecl* decl)
      : cache_(cache), decl_(decl) {}

  /// Attempt to generate marked source using the original source code.
  std::optional<MarkedSource> GenerateMarkedSourceUsingSource(
      const GraphObserver::NodeId& decl_id);

  /// Generate marked source for this decl, which must be a NamedDecl.
  MarkedSource GenerateMarkedSourceForNamedDecl();

  /// \brief Fill the provided MarkedSource node with the template argument list
  /// from `decl`.
  ///
  /// We want to label which template arguments have their default values.
  /// This is so that we don't print (e.g.) `std::vector<T, std::allocator<T>>`
  /// where most people only want to see `std::vector<T>`. Clang doesn't keep
  /// track of enough information by default.
  ///
  /// We cast the problem as finding the cutoff point in a template argument
  /// list such that if that argument and all subsequent arguments are dropped,
  /// the typechecker will infer a structurally equal argument list. For
  /// example, if we ask the typechecker to elaborate `std::vector<T>`,
  /// it will return the argument list `T, std::allocator<T>`. Since this is
  /// structurally equivalent to the original argument list, we know that
  /// we can drop the second argument without losing useful information.
  ///
  /// Because this operation is expensive, we cache (for each `decl`) its
  /// cutoff point. This cache is valid only for a particular AST.
  void ReplaceMarkedSourceWithTemplateArgumentList(
      MarkedSource* marked_source_node,
      const clang::ClassTemplateSpecializationDecl* decl);

  /// \brief Fill the provided MarkedSource node with the decl's annotated
  /// qualified name.
  ///
  /// This should typically replace the text range covered by the bare
  /// identifier in a normal decl (and any qualified name in an external decl).
  /// \return false on failure
  bool ReplaceMarkedSourceWithQualifiedName(MarkedSource* marked_source_node);

  /// The cache to consult (and to use to get at ::Sema and friends).
  MarkedSourceCache* cache_;

  /// The decl we're trying to build marked source for.
  const clang::NamedDecl* decl_;

  /// The end of the marked source-relevant portion of source code.
  clang::SourceLocation end_loc_;

  /// The range covering the decl's unqualified name.
  clang::SourceRange name_range_;

  /// The decl in question is implicit.
  bool implicit_ = false;
};

/// \brief Caches marked source generation information.
///
/// Cached values are tied to a particular AST and `Sema` instance.
class MarkedSourceCache {
 public:
  MarkedSourceCache(clang::Sema* sema, GraphObserver* graph_observer)
      : source_manager_(*graph_observer->getSourceManager()),
        lang_options_(*graph_observer->getLangOptions()),
        sema_(sema),
        observer_(graph_observer) {}

  /// \brief Return a new `MarkedSourceGenerator` for the given decl.
  MarkedSourceGenerator Generate(const clang::NamedDecl* decl) {
    return MarkedSourceGenerator(this, decl);
  }

  const clang::SourceManager& source_manager() const { return source_manager_; }
  const clang::LangOptions& lang_options() const { return lang_options_; }
  clang::Sema* sema() { return sema_; }
  GraphObserver* observer() { return observer_; }
  llvm::DenseMap<const clang::ClassTemplateSpecializationDecl*, unsigned>*
  first_default_template_argument() {
    return &first_default_template_argument_;
  }

 private:
  const clang::SourceManager& source_manager_;
  const clang::LangOptions& lang_options_;
  clang::Sema* sema_;
  GraphObserver* observer_;

  /// Maps from class template specializations to the first of that
  /// specialization's arguments that is default.
  llvm::DenseMap<const clang::ClassTemplateSpecializationDecl*, unsigned>
      first_default_template_argument_;
};
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_MARKED_SOURCE_H_
