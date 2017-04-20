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

#ifndef KYTHE_CXX_COMMON_PATH_UTILS_H_
#define KYTHE_CXX_COMMON_PATH_UTILS_H_

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"

#include <string>

namespace clang {
class FileEntry;
class Preprocessor;
}  // namespace clang

namespace kythe {
/// \brief Relativize `to_relativize` with respect to `relativize_against`.
///
/// If `to_relativize` does not name a path that is a child of
/// `relativize_against`, `RelativizePath` will return an absolute path.
///
/// \param to_relativize Relative or absolute path to a file.
/// \param relativize_against Relative or absolute path to a directory.
std::string RelativizePath(const std::string &to_relativize,
                           const std::string &relativize_against);

/// \brief Convert `in_path` to an absolute path, eliminating `.` and `..`.
/// \param in_path The path to convert.
std::string MakeCleanAbsolutePath(const std::string &in_path);

/// \brief Lexically eliminate `.` and `..` from `in_path`.
///
/// This function ignores the effects of symlinks.
///
/// \param in_path The path to convert.
std::string CleanPath(llvm::StringRef in_path);

/// \brief Append path `b` to path `a`, cleaning and returning the result.
std::string JoinPath(llvm::StringRef a, llvm::StringRef b);

/// \brief Looks up a file for an #include-ish pragma.
/// \param preprocessor The preprocessor to use to consume the filename tokens.
/// \param search_path The path used to find the file in the filesystem.
/// \param relative_path The path to the file, relative to search_path.
/// \param filename The filename used to consult the filesystem.
/// \return The FileEntry we found or null if we didn't find one.
const clang::FileEntry *LookupFileForIncludePragma(
    clang::Preprocessor *preprocessor, llvm::SmallVectorImpl<char> *search_path,
    llvm::SmallVectorImpl<char> *relative_path,
    llvm::SmallVectorImpl<char> *filename);
}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_PATH_UTILS_H_
