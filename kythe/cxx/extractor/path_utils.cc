/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

#include "path_utils.h"

#include <cstdio>

#include "absl/strings/str_format.h"
#include "clang/Basic/FileEntry.h"
#include "clang/Basic/TokenKinds.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Token.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Path.h"

namespace kythe {
namespace cxx_extractor {

const clang::FileEntry* LookupFileForIncludePragma(
    clang::Preprocessor* preprocessor, llvm::SmallVectorImpl<char>* search_path,
    llvm::SmallVectorImpl<char>* relative_path,
    llvm::SmallVectorImpl<char>* result_filename) {
  clang::Token filename_token;
  if (preprocessor->LexHeaderName(filename_token)) {
    return nullptr;
  }
  if (!filename_token.isOneOf(clang::tok::header_name,
                              clang::tok::string_literal)) {
    return nullptr;
  }
  llvm::SmallString<128> filename_buffer;
  llvm::StringRef filename =
      preprocessor->getSpelling(filename_token, filename_buffer);
  bool is_angled = preprocessor->GetIncludeFilenameSpelling(
      filename_token.getLocation(), filename);
  if (filename.empty()) {
    preprocessor->DiscardUntilEndOfDirective();
    return nullptr;
  }
  preprocessor->CheckEndOfDirective("pragma", true);
  if (preprocessor->getHeaderSearchInfo().HasIncludeAliasMap()) {
    auto mapped =
        preprocessor->getHeaderSearchInfo().MapHeaderToIncludeAlias(filename);
    if (!mapped.empty()) {
      filename = mapped;
    }
  }
  llvm::SmallString<1024> normalized_path;
  if (preprocessor->getLangOpts().MSVCCompat) {
    normalized_path = filename.str();
#ifndef LLVM_ON_WIN32
    llvm::sys::path::native(normalized_path);
#endif
    *result_filename = normalized_path;
  } else {
    result_filename->append(filename.begin(), filename.end());
  }
  clang::OptionalFileEntryRef file = preprocessor->LookupFile(
      filename_token.getLocation(),
      preprocessor->getLangOpts().MSVCCompat ? normalized_path.c_str()
                                             : filename,
      is_angled, nullptr /* FromDir */, nullptr /* FromFile */,
      nullptr /* CurDir */, search_path, relative_path,
      nullptr /* SuggestedModule */, nullptr /* IsMapped */,
      nullptr /* IsFrameworkFound */, false /* SkipCache */);
  if (!file) {
    absl::FPrintF(stderr, "Missing required file %s.\n", filename.str());
    return nullptr;
  }
  return &file->getFileEntry();
}

}  // namespace cxx_extractor
}  // namespace kythe
