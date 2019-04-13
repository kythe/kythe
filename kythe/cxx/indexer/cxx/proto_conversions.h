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

#ifndef KYTHE_CXX_INDEXER_CXX_PROTO_CONVERSIONS_H_
#define KYTHE_CXX_INDEXER_CXX_PROTO_CONVERSIONS_H_

#include <string>

#include "llvm/ADT/StringRef.h"

namespace kythe {
/// \brief Wrap a protobuf string in a StringRef.
/// \param string The string to wrap.
/// \return The wrapped string (which should not outlive `string`).
inline llvm::StringRef ToStringRef(const google::protobuf::string& string) {
  return llvm::StringRef(string.c_str(), string.size());
}
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_PROTO_CONVERSIONS_H_
