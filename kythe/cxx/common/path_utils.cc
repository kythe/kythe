/*
 * Copyright 2015 Google Inc. All rights reserved.
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

#include "clang/Basic/FileManager.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/LexDiagnostic.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/Path.h"
#include "path_utils.h"

namespace kythe {
std::string CleanPath(llvm::StringRef in_path) {
  llvm::SmallString<1024> out_path = in_path;
  // NB: This becomes llvm::sys::path::remove_dots.
  clang::FileManager::removeDotPaths(out_path, true);
  return out_path.str();
}

std::string JoinPath(llvm::StringRef a, llvm::StringRef b) {
  llvm::SmallString<1024> out_path = a;
  llvm::sys::path::append(out_path, b);
  return out_path.str();
}

std::string MakeCleanAbsolutePath(const std::string& in_path) {
  std::string abs_path = clang::tooling::getAbsolutePath(in_path);
  return CleanPath(abs_path);
}

std::string RelativizePath(const std::string& to_relativize,
                           const std::string& relativize_against) {
  std::string to_relativize_abs = MakeCleanAbsolutePath(to_relativize);
  std::string relativize_against_abs =
      MakeCleanAbsolutePath(relativize_against);
  llvm::StringRef to_relativize_parent =
      llvm::sys::path::parent_path(to_relativize_abs);
  std::string ret =
      to_relativize_parent.startswith(relativize_against_abs)
          ? to_relativize_abs.substr(relativize_against_abs.size() +
                                     llvm::sys::path::get_separator().size())
          : to_relativize_abs;
  return ret;
}

const clang::FileEntry* LookupFileForIncludePragma(
    clang::Preprocessor* preprocessor, llvm::SmallVectorImpl<char>* search_path,
    llvm::SmallVectorImpl<char>* relative_path,
    llvm::SmallVectorImpl<char>* result_filename) {
  clang::Token filename_token;
  clang::SourceLocation filename_end;
  llvm::StringRef filename;
  llvm::SmallString<128> filename_buffer;
  preprocessor->getCurrentLexer()->LexIncludeFilename(filename_token);
  switch (filename_token.getKind()) {
    case clang::tok::eod:
      return nullptr;
    case clang::tok::angle_string_literal:
    case clang::tok::string_literal:
      filename = preprocessor->getSpelling(filename_token, filename_buffer);
      break;
    case clang::tok::less:
      filename_buffer.push_back('<');
      if (preprocessor->ConcatenateIncludeName(filename_buffer, filename_end))
        return nullptr;
      filename = filename_buffer;
      break;
    default:
      preprocessor->DiscardUntilEndOfDirective();
      fprintf(stderr, "Bad include-style pragma.\n");
      return nullptr;
  }
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
  const clang::DirectoryLookup* cur_dir = nullptr;
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
  const clang::FileEntry* file = preprocessor->LookupFile(
      filename_token.getLocation(),
      preprocessor->getLangOpts().MSVCCompat ? normalized_path.c_str()
                                             : filename,
      is_angled, nullptr /* FromDir */, nullptr /* FromFile */, cur_dir,
      search_path, relative_path, nullptr /* SuggestedModule */,
      false /* SkipCache */);
  if (!file) {
    preprocessor->Diag(filename_token, clang::diag::err_pp_file_not_found)
        << filename;
  }
  return file;
}

}  // namespace kythe
