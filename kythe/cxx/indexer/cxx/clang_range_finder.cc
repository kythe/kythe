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

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Lexer.h"
#include "glog/logging.h"
#include "kythe/cxx/indexer/cxx/clang_utils.h"
#include "llvm/Support/raw_ostream.h"

namespace kythe {
namespace {
using ::clang::dyn_cast;
using ::clang::SourceLocation;
using ::clang::SourceRange;

std::string DumpString(const clang::Decl& decl) {
  std::string result;
  llvm::raw_string_ostream out(result);
  decl.dump(out);
  return out.str();
}
}  // namespace

SourceRange ClangRangeFinder::RangeForEntityAt(SourceLocation start) const {
  // TODO(shahms): Move the implementations of these here.
  return RangeForASTEntityFromSourceLocation(source_manager(), lang_options(),
                                             start);
}

SourceRange ClangRangeFinder::RangeForTokenAt(SourceLocation start) const {
  return ExpansionRange(SourceRange(start, start));
}

SourceRange ClangRangeFinder::RangeForNameOf(
    const clang::NamedDecl* decl) const {
  if (decl == nullptr) {
    return SourceRange();
  }
  if (auto dtor = dyn_cast<clang::CXXDestructorDecl>(decl)) {
    return RangeForNameOfDtor(*dtor);
  } else if (auto alias = dyn_cast<clang::ObjCCompatibleAliasDecl>(decl)) {
    return RangeForNameOfAlias(*alias);
  } else if (auto method = dyn_cast<clang::ObjCMethodDecl>(decl)) {
    return RangeForNameOfMethod(*method);
  } else if (auto ns = dyn_cast<clang::NamespaceDecl>(decl)) {
    return RangeForNamespace(*ns);
  }
  return RangeForEntityAt(decl->getLocation());
}

SourceRange ClangRangeFinder::RangeForNameOfDtor(
    const clang::CXXDestructorDecl& decl) const {
  if (SourceLocation loc = decl.getLocation();
      loc.isValid() && loc.isFileID()) {
    // TODO(shahms): The prior implementation checked this token against
    // the name of the class and only used it if they matched.
    // Do we want to replicate that or not?
    if (auto next = clang::Lexer::findNextToken(loc, source_manager(),
                                                lang_options())) {
      return SourceRange(loc, next->getEndLoc());
    }
  }
  return RangeForEntityAt(decl.getLocation());
}

SourceRange ClangRangeFinder::RangeForNameOfAlias(
    const clang::ObjCCompatibleAliasDecl& decl) const {
  // Ugliness to get the ranges for the tokens in this decl since clang does
  // not give them to us. We expect something of the form:
  // @compatibility_alias AliasName OriginalClassName
  // But all of the locations stored in the decl point to @ alone.
  if (auto loc = decl.getLocation(); loc.isValid() && loc.isFileID()) {
    // Use an offset because the immediately following token is
    // "compatibility_alias" and we want the name.
    if (auto next = clang::Lexer::findNextToken(
            loc.getLocWithOffset(1), source_manager(), lang_options())) {
      return SourceRange(next->getLocation(), next->getEndLoc());
    }
  }
  return RangeForEntityAt(decl.getLocation());
}

SourceRange ClangRangeFinder::RangeForNameOfMethod(
    const clang::ObjCMethodDecl& decl) const {
  // TODO(salguarnieri): Return multiple ranges, one for each portion of the
  // selector.
  if (decl.getNumSelectorLocs() > 0) {
    if (auto loc = decl.getSelectorLoc(0); loc.isValid() && loc.isFileID()) {
      // Only take the first selector (if we have one). This simplifies what
      // consumers of this data have to do but it is not correct.
      return RangeForTokenAt(loc);
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
  LOG(ERROR) << "Unable to determine source range for: " << DumpString(decl);
  return decl.getSourceRange();
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
    return RangeForTokenAt(decl.getBeginLoc());
  }
  return RangeForEntityAt(decl.getLocation());
}

SourceRange ClangRangeFinder::ExpansionRange(SourceRange range) const {
  return clang::Lexer::getAsCharRange(source_manager().getExpansionRange(range),
                                      source_manager(), lang_options())
      .getAsRange();
}
}  // namespace kythe
