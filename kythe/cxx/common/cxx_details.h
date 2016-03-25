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

#ifndef KYTHE_CXX_COMMON_CXX_DETAILS_H_
#define KYTHE_CXX_COMMON_CXX_DETAILS_H_

#include "clang/Basic/SourceManager.h"

#include <vector>

namespace kythe {
/// \brief Reproduces Clang's internal header search state.
struct HeaderSearchInfo {
  /// An include path to be searched.
  struct Path {
    /// The path to search.
    std::string path;
    /// Whether files in this path are normal, system, or implicitly extern C
    /// headers.
    clang::SrcMgr::CharacteristicKind characteristic_kind;
    /// Whether this Path is a framework.
    bool is_framework;
  };
  /// The first of the paths that is an <include>.
  unsigned angled_dir_idx = 0;
  /// The first of the system include paths. Must be >= angled_dir_idx.
  unsigned system_dir_idx = 0;
  /// Include paths to be searched, along with the kind of files found there.
  std::vector<Path> paths;
  /// Prefixes on include paths that override the system property.
  /// The second part of the pair determines whether the property is set.
  std::vector<std::pair<std::string, bool>> system_prefixes;
};

/// The type URI for C++ details.
extern const char kCxxCompilationUnitDetailsURI[];
}

#endif  // KYTHE_CXX_COMMON_CXX_DETAILS_H_
