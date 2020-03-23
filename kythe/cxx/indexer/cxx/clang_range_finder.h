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

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclarationName.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "glog/logging.h"

namespace kythe {

/// \brief Class responsible for finding suitable ranges for entities.
///
/// All valid ranges correspond to a concrete character-based location in a file
/// which is most suitable location to attribute to that entity.
class ClangRangeFinder {
 public:
  explicit ClangRangeFinder(const clang::SourceManager* source_manager,
                            const clang::LangOptions* lang_options)
      : source_manager_(CHECK_NOTNULL(source_manager)),
        lang_options_(CHECK_NOTNULL(lang_options)) {}

  /// \brief Returns a suitable range for the entity beginning at `start`.
  clang::SourceRange RangeForEntityAt(clang::SourceLocation start) const;
  /// \brief Returns a range for the single token beginning at `start`.
  clang::SourceRange RangeForTokenAt(clang::SourceLocation start) const;
  /// \brief Returns a range encompassing the name of the decl.
  clang::SourceRange RangeForNameOf(const clang::NamedDecl* decl) const;

  /// \brief Retruns a range of characters between start and end in the ultimate
  /// file.
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

  // TODO(shahms): If possible, treat all "anonymous" declarations alike.
  clang::SourceRange RangeForNamespace(const clang::NamespaceDecl& decl) const;

  /// \brief Converts a CharSourceRange to a character-oriented SourceRange.
  clang::SourceRange ToCharRange(clang::CharSourceRange range) const;

  const clang::SourceManager* source_manager_;
  const clang::LangOptions* lang_options_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_CLANG_RANGE_FINDER_H_
