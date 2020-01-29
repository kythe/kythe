/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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
#ifndef KYTHE_CXX_COMMON_VNAME_ORDERING_H_
#define KYTHE_CXX_COMMON_VNAME_ORDERING_H_

#include "kythe/proto/storage.pb.h"

namespace kythe {

/// \brief Defines equality on VNames by pairwise comparison of each vector
/// component.
template <typename VName>
bool VNameEquals(const VName& lhs, const VName& rhs) {
  return lhs.signature() == rhs.signature() && lhs.corpus() == rhs.corpus() &&
         lhs.root() == rhs.root() && lhs.path() == rhs.path() &&
         lhs.language() == rhs.language();
}

/// \brief Defines less-than on VNames as a lexicographic ordering on each
/// vector component.
struct VNameLess {
  template <typename VName>
  bool operator()(const VName& lhs, const VName& rhs) const {
    if (lhs.signature() < rhs.signature()) return true;
    if (lhs.signature() != rhs.signature()) return false;
    if (lhs.corpus() < rhs.corpus()) return true;
    if (lhs.corpus() != rhs.corpus()) return false;
    if (lhs.path() < rhs.path()) return true;
    if (lhs.path() != rhs.path()) return false;
    if (lhs.root() < rhs.root()) return true;
    if (lhs.root() != rhs.root()) return false;
    return lhs.language() < rhs.language();
  }
};
}  // namespace kythe

#endif
