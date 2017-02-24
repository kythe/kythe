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

#include "kythe/cxx/indexer/cxx/clang_utils.h"

#include "clang/AST/DeclTemplate.h"
#include "clang/Basic/CharInfo.h"
#include "clang/Lex/Lexer.h"
#include "glog/logging.h"

namespace kythe {
bool isObjCSelector(const clang::DeclarationName &DN) {
  switch (DN.getNameKind()) {
    case clang::DeclarationName::NameKind::ObjCZeroArgSelector:
    case clang::DeclarationName::NameKind::ObjCOneArgSelector:
    case clang::DeclarationName::NameKind::ObjCMultiArgSelector:
      return true;
    default:
      return false;
  }
}

void SkipWhitespace(const clang::SourceManager &source_manager,
                    clang::SourceLocation *loc) {
  CHECK(loc != nullptr);

  while (clang::isWhitespace(*source_manager.getCharacterData(*loc))) {
    *loc = loc->getLocWithOffset(1);
  }
}

clang::SourceRange RangeForASTEntityFromSourceLocation(
    const clang::SourceManager &source_manager,
    const clang::LangOptions &lang_options,
    clang::SourceLocation start_location) {
  if (start_location.isFileID()) {
    clang::SourceRange first_token_range =
        RangeForSingleTokenFromSourceLocation(source_manager, lang_options,
                                              start_location);
    llvm::SmallVector<char, 32> buffer;
    // TODO(jdennett): Prefer to lex a token and then check if it is
    // `clang::tok::kw_operator`, instead of checking the spelling?
    // If we do that, update `RangeForOperatorName` to take a `clang::Token`
    // as its argument?
    llvm::StringRef token_spelling = clang::Lexer::getSpelling(
        start_location, buffer, source_manager, lang_options);
    if (token_spelling == "operator") {
      return RangeForOperatorName(source_manager, lang_options,
                                  first_token_range);
    } else {
      return first_token_range;
    }
  } else {
    // The location is from macro expansion. We always need to return a
    // location that can be associated with the original file.
    clang::SourceLocation file_loc = source_manager.getFileLoc(start_location);
    if (IsTopLevelNonMacroMacroArgument(source_manager, start_location)) {
      // TODO(jdennett): Test cases such as `MACRO(operator[])`, where the
      // name in the macro argument should ideally not just be linked to a
      // single token.
      return RangeForSingleTokenFromSourceLocation(source_manager, lang_options,
                                                   file_loc);
    } else {
      // The entity is in a macro expansion, and it is not a top-level macro
      // argument that itself is not expanded from a macro. The range
      // is a 0-width span (a point), so no source link will be created.
      return clang::SourceRange(file_loc);
    }
  }
}

clang::SourceRange RangeForOperatorName(
    const clang::SourceManager &source_manager,
    const clang::LangOptions &lang_options,
    const clang::SourceRange &operator_token_range) {
  // Make a longer range than `operator_token_range`, if possible, so that
  // the range captures which operator this is.  There are two kinds of
  // operators; type conversion operators, and overloaded operators.
  // Most of the overloaded operators have names `operator ???` for some
  // token `???`, but there are exceptions (namely: `operator[]`,
  // `operator()`, `operator new[]` and `operator delete[]`.
  //
  // If this is a conversion operator to some type T, we link from the
  // `operator` keyword only, but if it's an overloaded operator then
  // we'd like to link from the name, e.g., `operator[]`, so long as
  // that is spelled out in the source file.
  clang::SourceLocation Pos = operator_token_range.getEnd();
  SkipWhitespace(source_manager, &Pos);

  // TODO(jdennett): Find a better name for `Token2EndLocation`, to
  // indicate that it's the end of the first token after `operator`.
  clang::SourceLocation Token2EndLocation =
      GetLocForEndOfToken(source_manager, lang_options, Pos);
  if (!Token2EndLocation.isValid()) {
    // Well, this is the best we can do then.
    return operator_token_range;
  }
  // TODO(jdennett): Prefer checking the token's type here also, rather
  // than the spelling.
  llvm::SmallVector<char, 32> Buffer;
  const clang::StringRef Token2Spelling =
      clang::Lexer::getSpelling(Pos, Buffer, source_manager, lang_options);
  // TODO(jdennett): Handle (and test) operator new, operator new[],
  // operator delete, and operator delete[].
  if (!Token2Spelling.empty() &&
      (Token2Spelling == "::" ||
       clang::Lexer::isIdentifierBodyChar(Token2Spelling[0], lang_options))) {
    // The token after `operator` is an identifier or keyword, or the
    // scope resolution operator `::`, so just return the range of
    // `operator` -- this is presumably a type conversion operator, and
    // the type-visiting code will add any appropriate links from the
    // type, so we shouldn't link from it here.
    return operator_token_range;
  }
  if (Token2Spelling == "(" || Token2Spelling == "[") {
    // TODO(jdennett): Do better than this disgusting hack to skip
    // whitespace.  We probably want to actually instantiate a lexer.
    clang::SourceLocation Pos = Token2EndLocation;
    SkipWhitespace(source_manager, &Pos);
    clang::Token Token3;
    if (clang::Lexer::getRawToken(Pos, Token3, source_manager, lang_options)) {
      // That's weird, we've got just part of the operator name -- we saw
      // an opening "(" or "]", but not its closing partner.  The best
      // we can do is to just return the range covering `operator`.
      llvm::errs() << "Failed to lex token after " << Token2Spelling.str();
      return operator_token_range;
    }

    if (Token3.is(clang::tok::r_square) || Token3.is(clang::tok::r_paren)) {
      clang::SourceLocation EndLocation = GetLocForEndOfToken(
          source_manager, lang_options, Token3.getLocation());
      return clang::SourceRange(operator_token_range.getBegin(), EndLocation);
    }
    return operator_token_range;
  }

  // In this case we assume we have `operator???`, for some single-token
  // operator name `???`.
  return clang::SourceRange(operator_token_range.getBegin(), Token2EndLocation);
}

clang::SourceLocation GetLocForEndOfToken(
    const clang::SourceManager &source_manager,
    const clang::LangOptions &lang_options,
    clang::SourceLocation start_location) {
  if (start_location.isMacroID()) {
    start_location = source_manager.getExpansionLoc(start_location);
  }
  return clang::Lexer::getLocForEndOfToken(start_location,
                                           0 /* offset from end of token */,
                                           source_manager, lang_options);
}

clang::SourceRange RangeForSingleTokenFromSourceLocation(
    const clang::SourceManager &source_manager,
    const clang::LangOptions &lang_options, clang::SourceLocation start) {
  CHECK(start.isFileID());
  clang::SourceLocation end =
      GetLocForEndOfToken(source_manager, lang_options, start);
  return clang::SourceRange(start, end);
}

bool IsTopLevelNonMacroMacroArgument(
    const clang::SourceManager &source_manager,
    const clang::SourceLocation &source_location) {
  if (!source_location.isMacroID()) return false;
  clang::SourceLocation loc = source_location;
  // We want to get closer towards the initial macro typed into the source only
  // if the location is being expanded as a macro argument.
  while (source_manager.isMacroArgExpansion(loc)) {
    // We are calling getImmediateMacroCallerLoc, but note it is essentially
    // equivalent to calling getImmediateSpellingLoc in this context according
    // to Clang implementation. We are not calling getImmediateSpellingLoc
    // because Clang comment says it "should not generally be used by clients."
    loc = source_manager.getImmediateMacroCallerLoc(loc);
  }
  return !loc.isMacroID();
}

const clang::Decl *FindSpecializedTemplate(const clang::Decl *decl) {
  if (const auto *FD = llvm::dyn_cast<const clang::FunctionDecl>(decl)) {
    if (auto *ftsi = FD->getTemplateSpecializationInfo()) {
      if (!ftsi->isExplicitInstantiationOrSpecialization()) {
        return ftsi->getTemplate();
      }
    }
  } else if (const auto *ctsd =
                 llvm::dyn_cast<const clang::ClassTemplateSpecializationDecl>(
                     decl)) {
    if (!ctsd->isExplicitInstantiationOrSpecialization()) {
      auto primary_or_partial = ctsd->getSpecializedTemplateOrPartial();
      if (const auto *partial =
              primary_or_partial.dyn_cast<
                  clang::ClassTemplatePartialSpecializationDecl *>()) {
        return partial;
      } else if (const auto *primary =
                     primary_or_partial
                         .dyn_cast<clang::ClassTemplateDecl *>()) {
        return primary;
      }
    }
  } else if (const auto *vtsd =
                 llvm::dyn_cast<const clang::VarTemplateSpecializationDecl>(
                     decl)) {
    if (!vtsd->isExplicitInstantiationOrSpecialization()) {
      auto primary_or_partial = vtsd->getSpecializedTemplateOrPartial();
      if (const auto *partial =
              primary_or_partial
                  .dyn_cast<clang::VarTemplatePartialSpecializationDecl *>()) {
        return partial;
      } else if (const auto *primary =
                     primary_or_partial.dyn_cast<clang::VarTemplateDecl *>()) {
        return primary;
      }
    }
  }
  return decl;
}
}  // namespace kythe
