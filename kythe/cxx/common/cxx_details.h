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
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "kythe/proto/cxx.pb.h"

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
  /// Copies HeaderSearchInfo from Clang. Returns true if we can represent the
  /// state; false if Clang is using features we don't support. This object
  /// has undefined state until the next successful CopyFrom completes.
  bool CopyFrom(const clang::HeaderSearchOptions& header_search_options,
                const clang::HeaderSearch& header_search_info);
  /// Copies HeaderSearchInfo from a serialized format. Returns true if the
  /// data are well-formed; false if this object should not be used and
  /// has undefined state until the next successful CopyFrom completes.
  bool CopyFrom(const kythe::proto::CxxCompilationUnitDetails& cxx_details);
  /// Copies HeaderSearchInfo to a serializable format.
  void CopyTo(kythe::proto::CxxCompilationUnitDetails* cxx_details) const;
};

/// The type URI for C++ details.
extern const char kCxxCompilationUnitDetailsURI[];
}

#endif  // KYTHE_CXX_COMMON_CXX_DETAILS_H_
