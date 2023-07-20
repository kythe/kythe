/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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
#ifndef KYTHE_CXX_INDEXER_CXX_CLANG_RANGE_FINDER_H_
#define KYTHE_CXX_INDEXER_CXX_CLANG_RANGE_FINDER_H_

#include "absl/log/die_if_null.h"
#include "absl/log/log.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclarationName.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"

namespace kythe {

/// \brief Class responsible for finding suitable ranges for entities.
///
/// All valid ranges correspond to a concrete character-based location in a file
/// which is most suitable location to attribute to that entity.
class ClangRangeFinder {
 public:
  /// \brief Constructs a new ClangRangeFinder using the provided SourceManager
  /// and LangOptions.
  explicit ClangRangeFinder(const clang::SourceManager* source_manager,
                            const clang::LangOptions* lang_options)
      : source_manager_(ABSL_DIE_IF_NULL(source_manager)),
        lang_options_(ABSL_DIE_IF_NULL(lang_options)) {}

  /// \brief Returns a range encompassing the name of the decl.
  clang::SourceRange RangeForNameOf(const clang::NamedDecl* decl) const;

  /// \brief Retruns a range of characters between start and end in the ultimate
  /// file. Notably, this resolves macro IDs to the either their expansion range
  /// or spelling range, depending on if the location is in an argument or body.
  /// This also expands the end location to include the characters comprising
  /// the final token.
  clang::SourceRange NormalizeRange(clang::SourceRange range) const;
  clang::SourceRange NormalizeRange(clang::SourceLocation start) const;
  clang::SourceRange NormalizeRange(clang::SourceLocation start,
                                    clang::SourceLocation end) const;

  const clang::SourceManager& source_manager() const {
    return *source_manager_;
  }
  const clang::LangOptions& lang_options() const { return *lang_options_; }

 private:
  clang::SourceRange RangeForNameInfo(
      const clang::DeclarationNameInfo& info) const;
  clang::SourceRange RangeForNameOfAlias(
      const clang::ObjCCompatibleAliasDecl& decl) const;
  clang::SourceRange RangeForNameOfMethod(
      const clang::ObjCMethodDecl& decl) const;
  clang::SourceRange RangeForNamespace(const clang::NamespaceDecl& decl) const;

  /// \brief Converts a CharSourceRange to a character-oriented SourceRange.
  clang::SourceRange ToCharRange(clang::CharSourceRange range) const;

  const clang::SourceManager* source_manager_;
  const clang::LangOptions* lang_options_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_CLANG_RANGE_FINDER_H_
