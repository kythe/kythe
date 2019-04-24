/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

#include <string>

#include "absl/strings/string_view.h"

namespace kythe {

/// \brief Append path `b` to path `a`, cleaning and returning the result.
std::string JoinPath(absl::string_view a, absl::string_view b);

/// \brief Collapse duplicate "/"s, resolve ".." and "." path elements, remove
/// trailing "/".
std::string CleanPath(absl::string_view input);

// \brief Return true if path is absolute.
bool IsAbsolutePath(absl::string_view path);

/// \brief Relativize `to_relativize` with respect to `relativize_against`.
///
/// If `to_relativize` does not name a path that is a child of
/// `relativize_against`, `RelativizePath` will return an absolute path
/// (resolved against the current working directory).
///
/// Note: arguments are assumed to be valid paths, but validity is not checked.
///
/// \param to_relativize Relative or absolute path to a file.
/// \param relativize_against Relative or absolute path to a directory.
std::string RelativizePath(absl::string_view to_relativize,
                           absl::string_view relativize_against);

/// \brief Convert `in_path` to an absolute path, eliminating `.` and `..`.
/// \param in_path The path to convert.
std::string MakeCleanAbsolutePath(absl::string_view in_path);

/// \brief Sets `*dir` to the process's current working directory.
///
/// On success, sets `*dir` and returns true.
///
/// On failure, returns false; `*dir` is unchanged but `errno` is set
/// to indicate the error as per `getcwd(3)`.
bool GetCurrentDirectory(std::string* dir);

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_PATH_UTILS_H_
