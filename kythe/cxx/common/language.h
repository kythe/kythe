/*
 * Copyright 2017 Google Inc. All rights reserved.
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

#ifndef KYTHE_CXX_COMMON_KYTHE_LANGUAGE_H_
#define KYTHE_CXX_COMMON_KYTHE_LANGUAGE_H_

#include <string>

#include "llvm/ADT/StringRef.h"

namespace kythe {
namespace supported_language {
enum class Language { kCpp, kObjectiveC };

/// \brief Return the enum version of s in lang and return true if s is a
/// supported language. If s is not a supported language, false is returned.
bool FromString(const std::string &s, Language *l);

std::string ToString(Language l);

/// \brief Returns a llvm::StringRef for the language. The StringRef will point
/// to data that never changes and will remain alive until the program exits.
llvm::StringRef ToStringRef(Language l);

};  // namespace supported_language
};  // namespace kythe

#endif  // KYTHE_CXX_COMMON_KYTHE_LANGUAGE_H_
