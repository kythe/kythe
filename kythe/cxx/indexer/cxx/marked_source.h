/*
 * Copyright 2016 Google Inc. All rights reserved.
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

#include "clang/AST/Decl.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Sema/Sema.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/indexing/MaybeFew.h"
#include "kythe/cxx/indexer/cxx/GraphObserver.h"

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
  void set_name_range(const clang::SourceRange &range) { name_range_ = range; }

  /// Attempt to build a marked source given all available information. Assumes
  /// that `decl_`'s ID is `decl_id`.
  MaybeFew<MarkedSource> GenerateMarkedSource(
      const GraphObserver::NodeId &decl_id);

 private:
  friend class MarkedSourceCache;

  MarkedSourceGenerator(MarkedSourceCache *cache, const clang::NamedDecl *decl)
      : cache_(cache), decl_(decl) {}

  /// The cache to consult (and to use to get at ::Sema and friends).
  MarkedSourceCache *cache_;

  /// The decl we're trying to build marked source for.
  const clang::NamedDecl *decl_;

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
  MarkedSourceCache(clang::Sema *sema, GraphObserver *graph_observer)
      : source_manager_(*graph_observer->getSourceManager()),
        lang_options_(*graph_observer->getLangOptions()),
        sema_(sema),
        observer_(graph_observer) {}

  /// \brief Return a new `MarkedSourceGenerator` for the given decl.
  MarkedSourceGenerator Generate(const clang::NamedDecl *decl) {
    return MarkedSourceGenerator(this, decl);
  }

  const clang::SourceManager &source_manager() const { return source_manager_; }
  const clang::LangOptions &lang_options() const { return lang_options_; }
  clang::Sema *sema() { return sema_; }
  GraphObserver *observer() { return observer_; }

 private:
  const clang::SourceManager &source_manager_;
  const clang::LangOptions &lang_options_;
  clang::Sema *sema_;
  GraphObserver *observer_;
};
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_MARKED_SOURCE_H_
