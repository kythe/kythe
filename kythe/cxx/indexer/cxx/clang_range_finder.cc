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
#include "kythe/cxx/indexer/cxx/clang_range_finder.h"

#include <string>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclarationName.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Lexer.h"
#include "kythe/cxx/indexer/cxx/stream_adapter.h"
#include "llvm/Support/raw_ostream.h"

namespace kythe {
namespace {
using ::clang::CharSourceRange;
using ::clang::dyn_cast;
using ::clang::SourceLocation;
using ::clang::SourceRange;

CharSourceRange GetImmediateFileRange(
    const clang::SourceManager& source_manager, const SourceLocation loc) {
  CHECK(!loc.isFileID());
  return source_manager.isMacroArgExpansion(loc)
             ? CharSourceRange::getTokenRange(
                   source_manager.getImmediateSpellingLoc(loc))
             : source_manager.getImmediateExpansionRange(loc);
}

CharSourceRange GetFileRange(CharSourceRange range,
                             const clang::SourceManager& source_manager,
                             const clang::LangOptions& lang_options) {
  bool begin_in_macro_body = false;
  bool begin_at_macro_start = false;
  while (!range.getBegin().isFileID()) {
    begin_in_macro_body = source_manager.isMacroBodyExpansion(range.getBegin());
    begin_at_macro_start = clang::Lexer::isAtStartOfMacroExpansion(
        range.getBegin(), source_manager, lang_options);
    range.setBegin(
        GetImmediateFileRange(source_manager, range.getBegin()).getBegin());
  }
  bool end_in_macro_body = false;
  bool end_at_macro_end = false;
  while (!range.getEnd().isFileID()) {
    end_in_macro_body = source_manager.isMacroBodyExpansion(range.getEnd());
    end_at_macro_end = clang::Lexer::isAtEndOfMacroExpansion(
        range.getEnd(), source_manager, lang_options);
    CharSourceRange end = GetImmediateFileRange(source_manager, range.getEnd());
    range.setEnd(end.getEnd());
    range.setTokenRange(end.isTokenRange());
  }

  // For macro body ranges we want to return a zero-width range
  // pointing to the beginning of the macro expansion so that any
  // edges originating from within the macro body are suppressed.
  // TODO(shahms): Emit the properly expanded range in all cases once UI
  // issues are addressed.
  if (
      // Only if both the immediately prior range is within a macro body for
      // both locations.
      (begin_in_macro_body && end_in_macro_body) &&
      // And only if both ends would not cover the entire macro expansion.
      // This covers the case where a macro expands to a single token, for
      // e.g. function renaming, etc.
      !(begin_at_macro_start && end_at_macro_end)) {
    return CharSourceRange::getCharRange(range.getBegin());
  }

  return range;
}

CharSourceRange GetFileRange(const SourceRange range,
                             const clang::SourceManager& source_manager,
                             const clang::LangOptions& lang_options) {
  return GetFileRange(CharSourceRange::getTokenRange(range), source_manager,
                      lang_options);
}
}  // namespace

SourceRange ClangRangeFinder::RangeForNameOf(
    const clang::NamedDecl* decl) const {
  if (decl == nullptr) {
    return SourceRange();
  }
  if (auto func = dyn_cast<clang::FunctionDecl>(decl)) {
    return RangeForNameInfo(func->getNameInfo());
  } else if (auto alias = dyn_cast<clang::ObjCCompatibleAliasDecl>(decl)) {
    return RangeForNameOfAlias(*alias);
  } else if (auto method = dyn_cast<clang::ObjCMethodDecl>(decl)) {
    return RangeForNameOfMethod(*method);
  } else if (auto ns = dyn_cast<clang::NamespaceDecl>(decl)) {
    return RangeForNamespace(*ns);
  }
  return NormalizeRange(decl->getLocation());
}

SourceRange ClangRangeFinder::RangeForNameInfo(
    const clang::DeclarationNameInfo& info) const {
  if (info.getName().getNameKind() ==
      clang::DeclarationName::CXXConversionFunctionName) {
    // For legacy reasons, in C++ conversion functions we intentionally omit the
    // type name and cover only the operator keyword.
    return NormalizeRange(info.getLoc());
  }
  return NormalizeRange(info.getSourceRange());
}

SourceRange ClangRangeFinder::RangeForNameOfAlias(
    const clang::ObjCCompatibleAliasDecl& decl) const {
  // Ugliness to get the ranges for the tokens in this decl since clang does
  // not give them to us. We expect something of the form:
  // @compatibility_alias AliasName OriginalClassName
  // But all of the locations stored in the decl point to @ alone.
  SourceLocation loc = decl.getLocation();
  if (loc.isValid() && loc.isFileID()) {
    // Use an offset because the immediately following token is
    // "compatibility_alias" and we want the name.
    if (auto next = clang::Lexer::findNextToken(
            loc.getLocWithOffset(1), source_manager(), lang_options())) {
      return SourceRange(next->getLocation(), next->getEndLoc());
    }
  }
  return NormalizeRange(loc);
}

SourceRange ClangRangeFinder::RangeForNameOfMethod(
    const clang::ObjCMethodDecl& decl) const {
  // TODO(salguarnieri): Return multiple ranges, one for each portion of the
  // selector.
  if (decl.getNumSelectorLocs() > 0) {
    if (auto loc = decl.getSelectorLoc(0); loc.isValid() && loc.isFileID()) {
      // Only take the first selector (if we have one). This simplifies what
      // consumers of this data have to do but it is not correct.
      return NormalizeRange(loc);
    }
  } else {
    // Take the whole declaration. For decls this goes up to but does not
    // include the ";". For definitions this goes up to but does not include
    // the "{". This range will include other fields, such as the return
    // type, parameter types, and parameter names.
    return SourceRange(decl.getSelectorStartLoc(), decl.getDeclaratorEndLoc());
  }

  // If the selector location is not valid or is not a file, return the
  // whole range of the selector and hope for the best.
  LOG(ERROR) << "Unable to determine source range for: "
             << StreamAdapter::Dump(decl);
  return NormalizeRange(decl.getSourceRange());
}

SourceRange ClangRangeFinder::RangeForNamespace(
    const clang::NamespaceDecl& decl) const {
  if (decl.isAnonymousNamespace()) {
    if (decl.isInline()) {
      if (auto next = clang::Lexer::findNextToken(
              decl.getBeginLoc(), source_manager(), lang_options())) {
        return SourceRange(next->getLocation(), next->getEndLoc());
      }
    }
    return NormalizeRange(decl.getBeginLoc());
  }
  return NormalizeRange(decl.getLocation());
}

SourceRange ClangRangeFinder::NormalizeRange(SourceLocation start) const {
  return ToCharRange(GetFileRange(start, source_manager(), lang_options()));
}

SourceRange ClangRangeFinder::NormalizeRange(SourceLocation start,
                                             SourceLocation end) const {
  return ToCharRange(
      GetFileRange(SourceRange(start, end), source_manager(), lang_options()));
}

SourceRange ClangRangeFinder::NormalizeRange(SourceRange range) const {
  return ToCharRange(GetFileRange(range, source_manager(), lang_options()));
}

SourceRange ClangRangeFinder::ToCharRange(clang::CharSourceRange range) const {
  return clang::Lexer::getAsCharRange(range, source_manager(), lang_options())
      .getAsRange();
}
}  // namespace kythe
