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

#ifndef KYTHE_CXX_INDEXER_CXX_CLANG_UTILS_H_
#define KYTHE_CXX_INDEXER_CXX_CLANG_UTILS_H_

#include "clang/AST/DeclarationName.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"

namespace kythe {
/// \return true if `DN` is an Objective-C selector.
bool isObjCSelector(const clang::DeclarationName &DN);

/// \brief Updates `*loc` to move it past whitespace characters.
///
/// This is useful because most of `clang::Lexer`'s static functions fail if
/// given a location that is in whitespace between tokens.
///
/// TODO(jdennett): Delete this if/when we replace its uses with sane lexing.
void SkipWhitespace(const clang::SourceManager &source_manager,
                    clang::SourceLocation *loc);

/// \brief Gets a suitable range for an AST entity from the `start_location`.
///
/// Note: if the AST entity is a declaration, use `RangeForNameOfDeclaration`,
/// as that can use more semantic information to determine a better range in
/// some cases.
///
/// The returned range is a best-effort attempt to cover the "name" of
/// the entity as written in the source code.
///
/// This range does double duty, being used for a "source link" as well as
/// to represent the semantic range of what's being used. Therefore,
/// if the entity is not in a macro expansion, or it comes from a top-level
/// macro argument and itself is not expanded from a macro, Kythe has found
/// it best to span just the entity's name -- in most cases, that's a
/// single token.  Otherwise, when the entity comes from a macro definition
/// (and not from a top-level macro argument), the resulting range is a
/// 0-width span (a point), so that no source link will be created.
///
/// Note: the definition of "suitable" is in the context of the AST visitor.
/// Being suitable may mean something different to the preprocessor.
clang::SourceRange RangeForASTEntityFromSourceLocation(
    const clang::SourceManager &source_manager,
    const clang::LangOptions &lang_options,
    clang::SourceLocation start_location);

/// \brief Gets a suitable range to represent the name of some `operator???`,
/// whether it's an overloaded operator or a conversion operator.
///
/// For conversion operators, the range is the given `operator_token_range`.
///
/// For overloaded operators, the range covers the whole operator name
/// (e.g., "operator++" or "operator[]").  Note: There's currently a bug
/// in that operators new/delete/new[]/delete[] get single-token ranges.
///
/// The argument `operator_token_range` should span the text of the
/// `operator` keyword.
clang::SourceRange RangeForOperatorName(
    const clang::SourceManager &source_manager,
    const clang::LangOptions &lang_options,
    const clang::SourceRange &operator_token_range);

/// \return the location at the end of the token starting at `start_location`
clang::SourceLocation GetLocForEndOfToken(
    const clang::SourceManager &source_manager,
    const clang::LangOptions &lang_options,
    clang::SourceLocation start_location);

/// \return the passed-in range with its end replaced by
/// GetLocForEndOfToken(old_end)
inline clang::SourceRange ExpandRangeBySingleToken(
    const clang::SourceManager &source_manager,
    const clang::LangOptions &lang_options,
    const clang::SourceRange &source_range) {
  return clang::SourceRange(
      source_range.getBegin(),
      GetLocForEndOfToken(source_manager, lang_options, source_range.getEnd()));
}

/// \brief Gets a range for a token from the location specified by
/// `start_location`.
///
/// Assumes the `start_location` is a file location (i.e., isFileID returning
/// true) because we want the range of a token in the context of the
/// original file.
clang::SourceRange RangeForSingleTokenFromSourceLocation(
    const clang::SourceManager &source_manager,
    const clang::LangOptions &lang_options,
    clang::SourceLocation start_location);

/// \brief Returns true if the `source_location` is from a top-level macro
/// argument and the argument itself is not expanded from a macro.
///
/// Example: returns true for `i` and false for `MACRO_INT_VAR` in the
/// following code segment.
///
/// ~~~
/// #define MACRO_INT_VAR i
/// int i;
/// MACRO_FUNCTION(i);              // a top level non-macro macro argument
/// MACRO_FUNCTION(MACRO_INT_VAR);  // a top level _macro_ macro argument
/// ~~~
bool IsTopLevelNonMacroMacroArgument(
    const clang::SourceManager &source_manager,
    const clang::SourceLocation &source_location);

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_CLANG_UTILS_H_
